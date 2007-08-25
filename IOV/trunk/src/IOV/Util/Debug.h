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

#ifndef _INF_DEBUG_H_
#define _INF_DEBUG_H_

#include <vpr/Util/Debug.h>


/** @name IOV Debug Categories */
//@{
/**
 * Debugging category for inf::Viewer.
 *
 * @since 0.26.4
 */
const vpr::DebugCategory infDBG_VIEWER(
   vpr::GUID("9bab82cf-2fb8-47dc-9f69-8a8343219ffc"), "DBG_INF_VIEWER",
   "infViewer:"
);

/**
 * Debugging category for inf::Plugin and subclasses thereof.
 *
 * @since 0.26.4
 */
const vpr::DebugCategory infDBG_PLUGIN(
   vpr::GUID("504a6441-6586-4bf2-acb9-73852d670f9c"), "DBG_INF_PLUGIN",
   "infPlugin:"
);
//@}


#endif /* _INF_DEBUG_H_ */
