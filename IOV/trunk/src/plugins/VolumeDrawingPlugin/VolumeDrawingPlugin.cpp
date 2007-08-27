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

#include <OpenSG/OSGConfig.h>

#include <boost/bind.hpp>
#include <boost/assign/list_of.hpp>

#include <vrkit/Viewer.h>
#include <vrkit/Version.h>
#include <vrkit/plugin/Creator.h>
#include <vrkit/plugin/Info.h>

#include "VolumeDrawingPlugin.h"


using namespace boost::assign;

static const vrkit::plugin::Info sInfo(
   "com.infiscape", "VolumeDrawingPlugin",
   list_of(VRKIT_VERSION_MAJOR)(VRKIT_VERSION_MINOR)(VRKIT_VERSION_PATCH)
);
static vrkit::plugin::Creator<vrkit::viewer::Plugin> sPluginCreator(
   boost::bind(&vrkit::VolumeDrawingPlugin::create, sInfo)
);

extern "C"
{

/** @name Plug-in Entry Points */
//@{
VRKIT_PLUGIN_API(const vrkit::plugin::Info*) getPluginInfo()
{
   return &sInfo;
}

VRKIT_PLUGIN_API(void) getPluginInterfaceVersion(vpr::Uint32& majorVer,
                                                 vpr::Uint32& minorVer)
{
   majorVer = VRKIT_PLUGIN_API_MAJOR;
   minorVer = VRKIT_PLUGIN_API_MINOR;
}

VRKIT_PLUGIN_API(vrkit::plugin::CreatorBase*) getCreator()
{
   return &sPluginCreator;
}
//@}
}

namespace vrkit
{

VolumeDrawingPlugin::VolumeDrawingPlugin(const plugin::Info& info)
   : viewer::Plugin(info)
{
   /* Do nothing. */ ;
}

viewer::PluginPtr VolumeDrawingPlugin::init(ViewerPtr)
{
   return shared_from_this();
}

void VolumeDrawingPlugin::contextInit(ViewerPtr viewer)
{
   viewer->getContextData().mRenderAction->setVolumeDrawing(isFocused());
}

void VolumeDrawingPlugin::update(ViewerPtr)
{
   /* Do nothing. */ ;
}

}
