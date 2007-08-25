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

#include <cstdlib>
#include <sstream>

#include <vpr/vpr.h>

#include <gmtl/Point.h>

#include <IOV/Util/Exceptions.h>
#include <IOV/Plugin/Module.h>
#include <IOV/Grab/GrabStrategy.h>


namespace inf
{

GrabStrategy::GrabStrategy(const inf::plugin::Info& info)
   : AbstractPlugin(info)
{
   /* Do nothing. */ ;
}

bool GrabStrategy::validatePluginLib(vpr::LibraryPtr pluginLib)
{
   inf::plugin::Module pm(pluginLib);

   AbstractPlugin::basicValidation(pm);

   const std::string get_version_func("getPluginInterfaceVersion");
   boost::function<void (vpr::Uint32&, vpr::Uint32&)> version_func =
      pm.getFunction<void (vpr::Uint32&, vpr::Uint32&)>(get_version_func);

   vpr::Uint32 major_ver;
   vpr::Uint32 minor_ver;
   version_func(major_ver, minor_ver);

   if ( major_ver != INF_GRAB_STRATEGY_PLUGIN_API_MAJOR )
   {
      std::ostringstream msg_stream;
      msg_stream << "Interface version mismatch: run-time does not match "
                 << "compile-time plug-in setting ("
                 << INF_GRAB_STRATEGY_PLUGIN_API_MAJOR << "."
                 << INF_GRAB_STRATEGY_PLUGIN_API_MINOR << " != " << major_ver
                 << "." << minor_ver << ")";
      throw inf::PluginInterfaceException(msg_stream.str(), IOV_LOCATION);
   }

   return true;
}

GrabStrategy::~GrabStrategy()
{
   /* Do nothing. */ ;
}

}
