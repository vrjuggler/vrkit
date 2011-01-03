// vrkit is (C) Copyright 2005-2011
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

#ifndef _VRKIT_PLUGIN_CREATOR_BASE_H_
#define _VRKIT_PLUGIN_CREATOR_BASE_H_

#include <vrkit/Config.h>

#include <boost/noncopyable.hpp>


namespace vrkit
{

namespace plugin
{

/** \class CreatorBase CreatorBase.h vrkit/plugin/CreatorBase.h
 *
 * Base class for plug-in creators. It does not actually create anything but
 * instead serves to allow derived types to be used in polymorphic contexts.
 * This class cannot be instantiated directly.
 *
 * @note This class was renamed from vrkit::PluginCreatorBase in version 0.47.
 *
 * @since 0.16.0
 */
class VRKIT_CLASS_API CreatorBase : private boost::noncopyable
{
protected:
   CreatorBase();

public:
   virtual ~CreatorBase();
};

}

}


#endif /* _VRKIT_PLUGIN_CREATOR_BASE_H_ */
