// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_SIGNAL_PROXY_H_
#define _INF_SIGNAL_PROXY_H_

#include <boost/signals/connection.hpp>


namespace inf
{

/**
 * Simple proxy for \c boost::signal<...> instances that exposes only the
 * connect() member function. This allows for a (somewhat) cleaner interface to
 * accessing a \c boost::signal<...> instance that is a class data member
 * without making that data member public. It is intended to be used for cases
 * when external classes need to connect a slot to a signal but the emitting
 * of the signal can only be performed by the holding class implementation.
 *
 * The idea for this class comes from the following article about signals and
 * slots:
 *
 * http://www.scottcollins.net/articles/a-deeper-look-at-signals-and-slots.html
 *
 * @since 0.24.1
 */
template<typename Signal>
class SignalProxy
{
public:
   typedef Signal signal_t;

   SignalProxy(signal_t& signal)
      : mSignal(signal)
   {
      /* Do nothing. */ ;
   }

   SignalProxy(const SignalProxy& rhs)
      : mSignal(rhs.mSignal)
   {
      /* Do nothing. */ ;
   }

   boost::signals::connection connect(typename signal_t::slot_type slot)
   {
      return mSignal.connect(slot);
   }

   /**
    * Connects the given slot to the contained signal using the identified
    * group.
    *
    * @param group The group in which the given slot will be connected.
    * @param slot  The slot to connect to this signal.
    *
    * @since 0.26.1
    */
   boost::signals::connection
   connect(const typename signal_t::group_type& group,
           typename signal_t::slot_type slot)
   {
      return mSignal.connect(group, slot);
   }

private:
   signal_t& mSignal;
};

}


#endif /* _INF_SIGNAL_PROXY_H_ */
