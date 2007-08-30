// vrkit is (C) Copyright 2005-2007
//    by Allen Bierbaum, Aron Bierbuam, Patrick Hartling, and Daniel Shipton
//
// This file is part of vrkit.
//
// vrkit is free software; you can redistribute it and/or modify it under the
// terms of the GNU Lesser General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// vrkit is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
// more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef _VRKIT_SIGNAL_EMITTER_H_
#define _VRKIT_SIGNAL_EMITTER_H_

#include <vrkit/Config.h>

#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/signal.hpp>
#include <boost/bind.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/mpl/for_each.hpp>


namespace vrkit
{

namespace signal
{

/**
 * A generic signal emitter. The signals are identified by a unique constant
 * as defined in the enumerated type \p SignalListContainer::signals. The range
 * of signals must be specified as the template parameters \p SignalBegin and
 * \p SignalEnd. The default values for these parameters are
 * \c SignalListContainer::BEGIN and \c SignalListContainer::END, meaning
 * that the identifiers for the emitted signals must be in the range
 * [BEGIN,END). If \p SignalListContainer::signals contains these identifiers,
 * then \c BEGIN ought to be 0 for the sake of efficiency. The value of \c END
 * must be 1 greater than the last value of \c SignalListContainer::signals.
 * This concept corresponds directly to that of an STL container with its
 * \c begin() and \c end() member functions that return iterators.
 *
 * The signature of the slot called when the signal is emitted matches
 * \p SlotSignature. See the declaration of \c signal_t for type declaration
 * of the boost::signal instance.
 *
 * @param SlotSignature       The slot function signature.
 * @param SignalListContainer An aggregate type containing a public enumerated
 *                            type named \c signals. This enum should define a
 *                            sequence of values in the range [BEGIN, END)
 *                            where \c BEGIN and \c END are among the values
 *                            defined. \c BEGIN ought to be 0 for the sake of
 *                            efficiency. The value of \c END must be 1
 *                            greater than the last value of
 *                            \c SignalListContainer::signals.
 * @param SignalBegin         The start of the range of signals. This defaults
 *                            to \c SignalListContainer::BEGIN. The value of
 *                            this member of the enum must be equal to the
 *                            first signal identifier in the enum (usually 0).
 * @param SignalEnd           The end marker for the range of signals. This
 *                            value must be one greater than the value of the
 *                            last signal identifier in the enum.
 *
 * @note The value of \c SignalListContainer::END indicates the number of
 *       signals that may be emitted by this signal emitter instantiation.
 *
 * @note This class was renamed from vrkit::SignalEmitter in version 0.47.
 *
 * @since 0.23.2
 */
template<typename SlotSignature
       , typename SignalListContainer
       , typename SignalListContainer::signals SignalBegin = SignalListContainer::BEGIN
       , typename SignalListContainer::signals SignalEnd = SignalListContainer::END>
class Emitter
{
public:
   typedef typename SignalListContainer::signals signal_list_t;

   typedef boost::signal<SlotSignature> signal_t;

   /**
    * Initializes the collection of signals that may be emitted herein.
    */
   Emitter()
      : mSignals(SignalEnd)
   {
      // Construct an MPL range of constants. \c SignalListContainer::signals
      // must be an enum, and its values must be in a sequential range
      // [BEGIN,END) (where BEGIN ought to be value 0). For example:
      //
      // enum signals
      // {
      //    BEGIN = 0,
      //    SIGNAL1 = 0,
      //    SIGNAL2,
      //    SIGNAL3,
      //    END
      // };
      typedef boost::mpl::range_c<
         signal_list_t, SignalBegin, SignalEnd
      > signals;

      // Iterate over the signals defined in S::signals and register each one.
      boost::mpl::for_each<signals>(boost::bind(&Emitter::addSignal, this,
                                    _1));
   }

   /**
    * Destructor.
    *
    * @post The \c boost::signal instances stored in \c mSignals are deleted.
    */
   ~Emitter()
   {
      /* Do nothing. */ ;
   }

   /**
    * Connects the given slot with the identified signal type. The connection
    * is returned, and the caller must retain a handle to this connection if
    * it will be necessary later to perform a manual signal/slot disconnection.
    *
    * @post The given slot is connected to the identified signal.
    *
    * @param signalType The identifier for the unique signal to which the
    *                   given slot will be connected.
    * @param slot       The slot to connect to the identified signal.
    *
    * @return The connection created between the signal and the slot is
    *         returned to the caller.
    */
   boost::signals::connection connect(const signal_list_t signalType,
                                      typename signal_t::slot_type slot)
   {
      return mSignals[signalType]->connect(slot);
   }

   /**
    * Connects the given slot with the identified signal type in the given
    * group. The connection is returned, and the caller must retain a handle
    * to this connection if it will be necessary later to perform a manual
    * signal/slot disconnection.
    *
    * @post The given slot is connected to the identified signal.
    *
    * @param signalType The identifier for the unique signal to which the
    *                   given slot will be connected.
    * @param group      The group to which the given slot will be added upon
    *                   connection.
    * @param slot       The slot to connect to the identified signal.
    *
    * @return The connection created between the signal and the slot is
    *         returned to the caller.
    */
   boost::signals::connection
   connect(const signal_list_t signalType,
           const typename signal_t::group_type& group,
           typename signal_t::slot_type slot)
   {
      return mSignals[signalType]->connect(group, slot);
   }

   /**
    * Retrieves a const reference to the identified signal. The usual purpose
    * of this would be to then emit the signal.
    *
    * @param signalType The identifier for the unique signal to which the
    *                   given slot will be connected.
    *
    * @return A const reference to the identified \c boost::signal instance is
    *         returned.
    */
   const signal_t& operator[](const signal_list_t signalType) const
   {
      return *mSignals[signalType];
   }

private:
   void addSignal(const signal_list_t signalType)
   {
      mSignals[signalType] = boost::shared_ptr<signal_t>(new signal_t());
   }

   std::vector< boost::shared_ptr<signal_t> > mSignals;
};

}

}


#endif /* _VRKIT_SIGNAL_EMITTER_H_ */
