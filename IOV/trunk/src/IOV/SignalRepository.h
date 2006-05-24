// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_SIGNAL_REPOSITORY_H_
#define _INF_SIGNAL_REPOSITORY_H_

#include <IOV/Config.h>

#include <map>
#include <boost/signal.hpp>
#include <boost/shared_ptr.hpp>

#include <vpr/vpr.h>
#include <vpr/Util/GUID.h>

#include <IOV/SceneData.h>
#include <IOV/SignalRepositoryPtr.h>


namespace inf
{

/**
 * Centralized storage for signals used for acitvating a new mode component.
 * Signals are identified and accessed by name (a string identifier), thereby
 * allowing an arbitrary number of signals to be used.
 *
 * @see inf::ModeComponent
 *
 * @since 0.23
 */
class IOV_CLASS_API SignalRepository : public inf::SceneData
{
public:
   /**
    * The unique type identifier for scene data used by plug-ins that manage
    * grabbing of objects in the scene.
    */
   static const vpr::GUID type_guid;

   static SignalRepositoryPtr create()
   {
      return SignalRepositoryPtr(new SignalRepository());
   }

   virtual ~SignalRepository();

   /**
    * The type for signals whose slots take a single argument of type
    * inf::SceneObjectPtr.
    */
   typedef boost::signal<void (const std::string&)> signal_t;

   /**
    * Adds a signal to correspond to the given identifier. If there is already
    * a signal known by the given identifier, an exception is thrown.
    *
    * @post The signal identified by \p id is a key in \c mSignals, and a
    *       new signal_t instance is the value for that key.
    *
    * @param id The identifier for the new signal to be added.
    *
    * @throw inf::Exception is thrown if an signal is already registered under
    *        the given identifier.
    */
   void addSignal(const std::string& id);

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
   bool hasSignal(const std::string& id) const;

   /**
    * Connects the given slot to the identified signal. If the signal was not
    * previously known, it is created. It is the responsibility of the code
    * making the connection to retain the returned
    * boost::signals::connection instance to allow for later disconnection.
    *
    * @post The signal identified by \p id is in \c mSignals. \p slot is
    *       connected to the identified signal.
    *
    * @param id   The identifier for the signal to which \p slot will be
    *             connected.
    * @param slot The slot to connect to the identified signal.
    *
    * @return A boost::signals::connection object is returned that the calling
    *         code can use for managing the connection between the signal and
    *         the given slot.
    */
   boost::signals::connection connect(const std::string& id,
                                      signal_t::slot_type slot);

   /**
    * Emits the identified signal.
    *
    * @pre \p id is a known signal.
    * @post All unblocked slots connected to the identified signal are
    *       invoked.
    *
    * @param id The identifier for the signal to be emitted.
    *
    * @throw inf::Exception is thrown if \p id does not identify a known
    *        signal.
    */
   void emit(const std::string& id);

protected:
   SignalRepository();

private:
   std::map<std::string, boost::shared_ptr<signal_t> > mSignals;
};

}


#endif /*_INF_SIGNAL_REPOSITORY_H_*/
