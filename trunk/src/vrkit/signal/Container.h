// vrkit is (C) Copyright 2005-2008
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

#ifndef _VRKIT_SIGNAL_CONTAINER_H_
#define _VRKIT_SIGNAL_CONTAINER_H_

#include <vrkit/Config.h>

#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <vrkit/signal/ContainerBase.h>


namespace vrkit
{

namespace signal
{

/** \class Container Container.h vrkit/signal/Container.h
 *
 * This is the real signal container type. vrkit::signal::ContainerBase is
 * used so that vrkit::signal::Repository can operate in terms of an abstract
 * base type without needing to know all possible instantiations of this
 * subclass. Code wishing to register a signal dynamically creates an
 * instantiation of this type using a signal type (such as an instantiation of
 * boost::signal) as the template parameter. An object of that instantiated
 * type is then registered with the signal repository using a unique name.
 *
 * This class acts as a smart pointer for the contained \c SignalType object.
 * Thus, it is a very thin wrapper around that type intended solely for
 * storing instances of \c SignalType in vrkit::signal::Repository. Calling
 * code simply derferences the signal container object to get access to the
 * contained signal.
 *
 * @see vrkit::signal::Repository
 *
 * @note This class was renamed from vrkit::SignalContainer in version 0.47.
 *
 * @since 0.34
 */
template<typename SignalType>
class Container
   : public ContainerBase
   , public boost::enable_shared_from_this< Container<SignalType> >
{
public:
   typedef boost::shared_ptr<Container> ptr_type;
   typedef SignalType                   signal_type;

protected:
   Container()
      : ContainerBase()
   {
      /* Do nothing. */ ;
   }

public:
   static ptr_type create()
   {
      return ptr_type(new Container());
   }

   virtual ~Container()
   {
      /* Do nothing. */ ;
   }

   //@{
   /** @name Smart Pointer Interface */
   signal_type* operator->()
   {
      return &mSignal;
   }

   signal_type& operator*()
   {
      return mSignal;
   }
   //@}

private:
   signal_type mSignal;
};

}

}


#endif /* _VRKIT_SIGNAL_CONTAINER_H_ */
