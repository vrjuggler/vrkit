// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_SIGNAL_CONTAINER_H_
#define _INF_SIGNAL_CONTAINER_H_

#include <IOV/Config.h>

#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <IOV/SignalContainerBase.h>


namespace inf
{

/**
 * This is the real signal container type. inf::SignalContainerBase is used so
 * that inf::SignalRepository can operate in terms of an abstract base type
 * without needing to know all possible instantiations of this subclass.
 * Code wishing to register a signal dynamically creates an instantiation of
 * this type using a signal type (such as an instantiation of boost::signal)
 * as the template parameter. An object of that instantiated type is then
 * registered with the signal repository using a unique name.
 *
 * This class acts as a smart pointer for the contained \c SignalType object.
 * Thus, it is a very thin wrapper around that type intended solely for
 * storing instances of \c SignalType in inf::SignalRepository. Calling code
 * simply derferences the signal container object to get access to the
 * contained signal.
 *
 * @see inf::SignalRepository
 *
 * @since 0.34
 */
template<typename SignalType>
class SignalContainer
   : public SignalContainerBase
   , public boost::enable_shared_from_this< SignalContainer<SignalType> >
{
public:
   typedef boost::shared_ptr<SignalContainer> ptr_type;

protected:
   SignalContainer()
      : SignalContainerBase()
   {
      /* Do nothing. */ ;
   }

public:
   static ptr_type create()
   {
      return ptr_type(new SignalContainer());
   }

   virtual ~SignalContainer()
   {
      /* Do nothing. */ ;
   }

   //@{
   /** @name Smart Pointer Interface */
   SignalType* operator->()
   {
      return &mSignal;
   }

   SignalType& operator*()
   {
      return mSignal;
   }
   //@}

private:
   SignalType mSignal;
};

}


#endif /* _INF_SIGNAL_CONTAINER_H_ */
