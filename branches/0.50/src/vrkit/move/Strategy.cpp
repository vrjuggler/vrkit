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

#include <vrkit/exceptions/PluginInterfaceException.h>
#include <vrkit/plugin/Module.h>
#include <vrkit/move/Strategy.h>


namespace vrkit
{

namespace move
{

Strategy::Strategy(const plugin::Info& info)
   : AbstractPlugin(info)
{
   /* Do nothing. */ ;
}

Strategy::~Strategy()
{
   /* Do nothing. */ ;
}

bool Strategy::validatePluginLib(vpr::LibraryPtr pluginLib)
{
   plugin::Module pm(pluginLib);

   AbstractPlugin::basicValidation(pm);

   const std::string get_version_func("getPluginInterfaceVersion");
   boost::function<void (vpr::Uint32&, vpr::Uint32&)> version_func =
      pm.getFunction<void (vpr::Uint32&, vpr::Uint32&)>(get_version_func);

   vpr::Uint32 major_ver;
   vpr::Uint32 minor_ver;
   version_func(major_ver, minor_ver);

   if ( major_ver != VRKIT_MOVE_STRATEGY_PLUGIN_API_MAJOR )
   {
      std::ostringstream msg_stream;
      msg_stream << "Interface version mismatch: run-time does not match "
                 << "compile-time plug-in setting ("
                 << VRKIT_MOVE_STRATEGY_PLUGIN_API_MAJOR << "."
                 << VRKIT_MOVE_STRATEGY_PLUGIN_API_MINOR << " != "
                 << major_ver << "." << minor_ver << ")";
      throw PluginInterfaceException(msg_stream.str(), VRKIT_LOCATION);
   }

   return true;
}

}

}
