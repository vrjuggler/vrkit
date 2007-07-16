// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_SIGNAL_REPOSITORY_H_
#define _INF_SIGNAL_REPOSITORY_H_

#include <IOV/Config.h>

#include <stdexcept>
#include <map>
#include <boost/shared_ptr.hpp>

#include <vpr/vpr.h>
#include <vpr/Util/GUID.h>

#include <IOV/SceneData.h>
#include <IOV/SignalContainer.h>
#include <IOV/SignalRepositoryPtr.h>


namespace inf
{

/**
 * Centrailzed storage for signals. This is typically used for inter-plug-in
 * communication where one plug-in emits a signal and one or more other
 * plug-ins are interested in receiving the signal. Signals are identified and
 * accessed by name (a string identifier defined by the code registering the
 * signal), thereby allowing an arbitrary number of signals to be used.
 *
 * This class was refactored in IOV 0.34 to allow for more flexible use of
 * signals. It is no longer necessary that all signals used with this
 * repository have a void return type and take a single const reference to a
 * std::string object. Now, the calling code determines what the signal
 * signature is to be. Moreover, this class does not instantiate the signal
 * objects itself. Instead, the calling code passes in a signal container
 * object that holds the actual signal object of interest. Calling code then
 * uses getSignal() or getBaseSignal() to get the signal container so it can
 * then utilize the signal contained therein.
 *
 * @see SignalContainerBase
 *
 * @since 0.23
 */
class IOV_CLASS_API SignalRepository : public inf::SceneData
{
protected:
   SignalRepository();

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
    * @throw inf::Exception is thrown if an signal is already registered under
    *        the given identifier.
    */
   void addSignal(const std::string& id, SignalContainerBasePtr container);

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
    * containers beyond inf::SignalContainer<T>. However, it is important to
    * understand that the signal repository is designed for use with
    * inf::SignalContainer<T>, and the results of trying to use some other
    * subclass of inf::SignalContainerBase may be sub-optimal.
    *
    * @pre The given identifier is for a registered signal container object.
    *
    * @param id The identifier for the signal container to be retrieved.
    *
    * @throw std::invalid_argument is thrown if \p id is not a registered
    *        signal.
    *
    * @since 0.34
    */
   SignalContainerBasePtr getBaseSignal(const std::string& id);

   /**
    * Retrieves the signal container associated with the given identifier.
    * The container is cast dynamically to an inf::SignalContainer<T>
    * instantiation where \c T matches the template parameter \c SignalType.
    *
    * @pre The given identifier is for a registered signal container object.
    *
    * @param id The identifier for the signal container to be retrieved.
    *
    * @throw std::invalid_argument is thrown if \p id is not a registered
    *        signal.
    * @throw std::bad_cast is thrown if the signal associated with the given
    *        identifier cannot be cast dynamically to a container for the
    *        given signal type.
    *
    * @note This method returns a reference to an instantiation of
    *       SignalContainer<SignalType>. If the calling code wishes to get
    *       something that can be retained beyond the scope of the function
    *       invoking this method, invoke shared_from_this() on the returned
    *       SignalContainer<SignalType> reference to retrieve a
    *       boost::shared_ptr< SignalContainer<SignalType> > object.
    *
    * @since 0.34
    */
   template<typename SignalType>
   SignalContainer<SignalType>& getSignal(const std::string& id)
   {
      typedef SignalContainer<SignalType> container_type;
      typename container_type::ptr_type container =
         boost::dynamic_pointer_cast<container_type>(getBaseSignal(id));

      if ( ! container )
      {
         throw std::bad_cast();
      }

      return *container;
   }

private:
   std::map<std::string, SignalContainerBasePtr> mSignals;
};

}


#endif /*_INF_SIGNAL_REPOSITORY_H_*/
