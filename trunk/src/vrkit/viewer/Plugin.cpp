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

#include <cstdlib>
#include <string>
#include <sstream>

#include <vpr/vpr.h>

#include <vrkit/exceptions/PluginInterfaceException.h>
#include <vrkit/plugin/Module.h>
#include <vrkit/viewer/Plugin.h>


namespace vrkit
{

namespace viewer
{

// NOTE: This is here in the .cpp file to ensure that vrkit::viewer::Plugin is
// compiled properly.  Should this code ever become part of a library, it is
// important to ensure that polymorphic types are compiled into the correct
// place.
Plugin::Plugin(const plugin::Info& info)
   : AbstractPlugin(info)
   , mIsFocused(false)
{
   /* Do nothing. */ ;
}

// NOTE: This is here in the .cpp file to ensure that vrkit::viewer::Plugin is
// compiled properly.  Should this code ever become part of a library, it is
// important to ensure that polymorphic types are compiled into the correct
// place.
Plugin::~Plugin()
{
   /* Do nothing. */ ;
}

bool Plugin::validatePluginLib(vpr::LibraryPtr pluginLib)
{
   plugin::Module pm(pluginLib);

   AbstractPlugin::basicValidation(pm);

   const std::string get_version_func("getPluginInterfaceVersion");
   boost::function<void (vpr::Uint32&, vpr::Uint32&)> version_func =
      pm.getFunction<void (vpr::Uint32&, vpr::Uint32&)>(get_version_func);

   vpr::Uint32 major_ver;
   vpr::Uint32 minor_ver;
   version_func(major_ver, minor_ver);

   if ( major_ver != VRKIT_PLUGIN_API_MAJOR )
   {
      std::ostringstream msg_stream;
      msg_stream << "Interface version mismatch: run-time does not match "
                 << "compile-time plug-in setting ("
                 << VRKIT_PLUGIN_API_MAJOR << "." << VRKIT_PLUGIN_API_MINOR
                 << " != " << major_ver << "." << minor_ver << ")";
      throw PluginInterfaceException(msg_stream.str(), VRKIT_LOCATION);
   }

   return true;
}

void Plugin::contextInit(ViewerPtr)
{
   /* Do nothing. */ ;
}

void Plugin::contextPreDraw(ViewerPtr)
{
   /* Do nothing. */ ;
}

void Plugin::draw(ViewerPtr)
{
   /* Do nothing. */ ;
}

void Plugin::contextPostDraw(ViewerPtr)
{
   /* Do nothing. */ ;
}

void Plugin::contextClose(ViewerPtr)
{
   /* Do nothing. */ ;
}

void Plugin::setFocused(ViewerPtr viewer, const bool focused)
{
   if ( mIsFocused != focused )
   {
      mIsFocused = focused;
      focusChanged(viewer);
   }
}

void Plugin::focusChanged(ViewerPtr)
{
   /* Do nothing. */ ;
}

}

}
