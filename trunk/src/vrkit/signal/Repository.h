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

#ifndef _VRKIT_SIGNAL_REPOSITORY_H_
#define _VRKIT_SIGNAL_REPOSITORY_H_

#include <vrkit/Config.h>

#include <stdexcept>
#include <map>
#include <boost/shared_ptr.hpp>

#include <vpr/vpr.h>
#include <vpr/Util/GUID.h>

#include <vrkit/SceneData.h>
#include <vrkit/signal/Container.h>
#include <vrkit/signal/RepositoryPtr.h>


namespace vrkit
{

namespace signal
{

/**
 * @example "Example of using vrkit::signal::Repository"
 *
 * The following shows how the vrkit Signal Repository can be used for dynamic
 * signal registration and retrieval.
 *
 * \code
 * typedef boost::signal<void (int)> sig_type;
 *
 * void register(vrkit::signal::RepositoryPtr repos)
 * {
 *    vrkit::signal::ContainerBasePtr c =
 *       vrkit::signal::Container<sig_type>::create();
 *    repos->addSignal("My Signal", c);
 * }
 *
 * void slot(int v)
 * {
 *    std::cout << "I received " << v << std::endl;
 * }
 *
 * void connect(vrkit::signal::RepositoryPtr repos)
 * {
 *    repos->getSignal<sig_type>("My Signal")->connect(&slot);
 * }
 *
 * void emit(vrkit::signal::RepositoryPtr repos, const int value)
 * {
 *    (*repos->getSignal<sig_type>("My Signal"))(value);
 * }
 *
 * void run()
 * {
 *    vrkit::signal::RepositoryPtr repos = vrkit::signal::Repository::create();
 *
 *    register(repos);
 *    connect(repos);
 *
 *    for ( int i = 0; i < 5; ++i )
 *    {
 *       emit(repos, i);
 *    }
 * }
 * \endcode
 *
 * The emit() function could also be implemented in this way:
 *
 * \code
 * void emit(vrkit::signal::RepositoryPtr repos, const int value)
 * {
 *    vrkit::signal::Container<sig_type> c = repos->getSignal("My Signal");
 *    sig_type& signal = *c;
 *    signal(value);
 * }
 * \endcode
 *
 * A more realistic usage would not have everything (registration, connection,
 * and emission) happen in one function (as is done in run() above). A more
 * likely sceanrio is that one plug-in would register a signal and another
 * would connect a slot to that signal by retrieving it from the Signal
 * Repository. Then, the first plug-in would emit signals, and the second
 * would have its slot invoked.
 *
 * Alternatively, the first plug-in would register the signal and connect a
 * slot of its own to it. Then, some other code (the application, a plug-in,
 * etc.) would get the signal container to emit the signal. The plug-in that
 * registered the signal would then have its slot invoked.
 *
 * In both of these scenarios, the vrkit::signal::Repository instance would be
 * retrieved as vrkit scene data. This is the standard vrkit mechanism for
 * plug-ins to get access to shared objects.
 *
 * @see vrkit::Scene
 */

/** \class Repository Repository.h vrkit/signal/Repository.h
 *
 * Centrailzed storage for signals. This is typically used for inter-plug-in
 * communication where one plug-in emits a signal and one or more other
 * plug-ins are interested in receiving the signal. Signals are identified and
 * accessed by name (a string identifier defined by the code registering the
 * signal), thereby allowing an arbitrary number of signals to be used.
 *
 * This class was refactored in vrkit 0.34 to allow for more flexible use of
 * signals. It is no longer necessary that all signals used with this
 * repository have a void return type and take a single const reference to a
 * std::string object. Now, the calling code determines what the signal
 * signature is to be. Moreover, this class does not instantiate the signal
 * objects itself. Instead, the calling code passes in a signal container
 * object that holds the actual signal object of interest. Calling code then
 * uses getSignal() or getBaseSignal() to get the signal container so it can
 * then utilize the signal contained therein.
 *
 * @see vrkit::signal::ContainerBase
 *
 * @note This class was renamed from vrkit::SignalRepository in version 0.47.
 *
 * @since 0.23
 */
class VRKIT_CLASS_API Repository : public SceneData
{
protected:
   Repository();

public:
   /**
    * The unique type identifier for scene data used by plug-ins that manage
    * grabbing of objects in the scene.
    */
   static const vpr::GUID type_guid;

   static RepositoryPtr create()
   {
      return RepositoryPtr(new Repository());
   }

   virtual ~Repository();

   /**
    * Adds a signal to correspond to the given identifier. If there is already
    * a signal known by the given identifier, an exception is thrown.
    *
    * @post The signal identified by \p id is a key in \c mSignals, and a
    *       new signal_t instance is the value for that key.
    *
    * @param id        The identifier for the new signal to be added.
    * @param container The container for the signal being added to the
    *                  repository.
    *
    * @throw vrkit::Exception
    *           Thrown if an signal is already registered under the given
    *           identifier.
    */
   void addSignal(const std::string& id, ContainerBasePtr container);

   /**
    * If there is no signal registered with the given identifier, then no
    * action is taken.
    */
   void removeSignal(const std::string& id);

   /**
    * Determines whether the identified signal is known.
    *
    * @param id The identifier for the signal to look up.
    *
    * @return \c true is returned if \p id is a known signal; otherwise,
    *         \c false is returned.
    */
   bool hasSignal(const std::string& id) const
   {
      return mSignals.count(id) > 0;
   }

   /**
    * Retrieves the signal container associated with the given identifier.
    * This method exists so that there is room to extend the use of signal
    * containers beyond vrkit::signal::Container<T>. However, it is important
    * to understand that the signal repository is designed for use with
    * vrkit::signal::Container<T>, and the results of trying to use some other
    * subclass of vrkit::signal::ContainerBase may be sub-optimal.
    *
    * @pre The given identifier is for a registered signal container object.
    *
    * @param id The identifier for the signal container to be retrieved.
    *
    * @throw std::invalid_argument Thrown if \p id is not a registered signal.
    *
    * @since 0.34
    */
   ContainerBasePtr getBaseSignal(const std::string& id);

   /**
    * Retrieves the signal container associated with the given identifier.
    * The container is cast dynamically to a vrkit::signal::Container<T>
    * instantiation where \c T matches the template parameter \c SignalType.
    *
    * @pre The given identifier is for a registered signal container object.
    *
    * @param id The identifier for the signal container to be retrieved.
    *
    * @throw std::invalid_argument
    *           Thrown if \p id is not a registered signal.
    * @throw std::bad_cast
    *           Thrown if the signal associated with the given identifier
    *           cannot be cast dynamically to a container for the given signal
    *           type.
    *
    * @note This method returns a reference to an instantiation of
    *       vrkit::signal::Container<SignalType>. If the calling code wishes
    *       to get something that can be retained beyond the scope of the
    *       function invoking this method, invoke shared_from_this() on the
    *       returned vrkit::signal::Container<SignalType> reference to
    *       retrieve a
    *       boost::shared_ptr< vrkit::signal::Container<SignalType> > object.
    *
    * @since 0.34
    */
   template<typename SignalType>
   Container<SignalType>& getSignal(const std::string& id)
   {
      typedef Container<SignalType> container_type;
      typename container_type::ptr_type container =
         boost::dynamic_pointer_cast<container_type>(getBaseSignal(id));

      if ( ! container )
      {
         throw std::bad_cast();
      }

      return *container;
   }

private:
   std::map<std::string, ContainerBasePtr> mSignals;
};

}

}


#endif /* _VRKIT_SIGNAL_REPOSITORY_H_ */
