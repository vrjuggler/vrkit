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

#ifndef _VRKIT_UTIL_DEBUG_H_
#define _VRKIT_UTIL_DEBUG_H_

#include <vpr/Util/Debug.h>


/** @name vrkit Debug Categories */
//@{
/**
 * Debugging category for vrkit::Viewer.
 *
 * @since 0.26.4
 */
const vpr::DebugCategory vrkitDBG_VIEWER(
   vpr::GUID("9bab82cf-2fb8-47dc-9f69-8a8343219ffc"), "DBG_VRKIT_VIEWER",
   "vrkitViewer:"
);

/**
 * Debugging category for vrkit::Plugin and subclasses thereof.
 *
 * @since 0.26.4
 */
const vpr::DebugCategory vrkitDBG_PLUGIN(
   vpr::GUID("504a6441-6586-4bf2-acb9-73852d670f9c"), "DBG_VRKIT_PLUGIN",
   "vrkitPlugin:"
);
//@}


#endif /* _VRKIT_UTIL_DEBUG_H_ */
