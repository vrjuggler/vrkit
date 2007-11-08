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

#include <sstream>

#include <vpr/Util/Assert.h>

#include <vrkit/plugin/Module.h>
#include <vrkit/plugin/Info.h>
#include <vrkit/exceptions/PluginInterfaceException.h>
#include <vrkit/AbstractPlugin.h>


namespace vrkit
{

AbstractPlugin::AbstractPlugin(const plugin::Info& info)
   : mInfo(info)
{
   ;
}

AbstractPlugin::~AbstractPlugin()
{
   ;
}

void AbstractPlugin::basicValidation(const plugin::Module& module)
{
   vprASSERT(module.getLibrary()->isLoaded() &&
             "Plug-in library is not loaded");

   const std::string get_info_func_name(getInfoFuncName());

   if ( ! module.hasSymbol(get_info_func_name) )
   {
      std::ostringstream msg_stream;
      msg_stream << "Plug-in '" << module.getLibrary()->getName()
                 << "' has no entry point function named "
                 << get_info_func_name;
      throw PluginInterfaceException(msg_stream.str(), VRKIT_LOCATION);
   }
}

}
