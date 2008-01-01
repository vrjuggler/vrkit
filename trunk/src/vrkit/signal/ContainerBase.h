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

#ifndef _VRKIT_SIGNAL_CONTAINER_BASE_H_
#define _VRKIT_SIGNAL_CONTAINER_BASE_H_

#include <vrkit/Config.h>

#include <boost/noncopyable.hpp>

#include <vrkit/signal/ContainerBasePtr.h>


namespace vrkit
{

namespace signal
{

/** \class ContainerBase ContainerBase.h vrkit/signal/ContainerBase.h
 *
 * The base class for all signal containers that can be registered with the
 * signal repository. For all intents and purposes, this is an abstract base
 * class, although it does not define any pure virtual methods. Instead, it
 * has a virtual destructor to ensure that this class has a vtable and can be
 * handled polymorphically.
 *
 * @see vrkit::signal::Repository
 *
 * @note This class was renamed from vrkit::SignalContainerBase in version
 *       0.47.
 *
 * @since 0.34
 */
class VRKIT_CLASS_API ContainerBase : private boost::noncopyable
{
protected:
   ContainerBase();

public:
   virtual ~ContainerBase();
};

}

}


#endif /* _VRKIT_SIGNAL_CONTAINER_BASE_H_ */
