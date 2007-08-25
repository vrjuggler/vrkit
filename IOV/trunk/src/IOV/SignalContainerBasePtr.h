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

#ifndef _INF_SIGNAL_CONTAINER_BASE_PTR_H_
#define _INF_SIGNAL_CONTAINER_BASE_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class SignalContainerBase;
   typedef boost::shared_ptr<SignalContainerBase> SignalContainerBasePtr;
   typedef boost::weak_ptr<SignalContainerBase> SignalContainerBaseWeakPtr;
}

#endif /*_INF_SIGNAL_CONTAINER_BASE_PTR_H_*/
