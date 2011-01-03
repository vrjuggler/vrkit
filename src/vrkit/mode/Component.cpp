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

#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <exception>

#include <vpr/vpr.h>
#include <vpr/Util/Assert.h>

#include <vrkit/exceptions/PluginInterfaceException.h>
#include <vrkit/mode/Component.h>


namespace vrkit
{

namespace mode
{

// NOTE: This is here in the .cpp file to ensure that vrkit::mode::Component
// is compiled properly.  Should this code ever become part of a library, it
// is important to ensure that polymorphic types are compiled into the correct
// place.
Component::Component(const plugin::Info& info)
   : AbstractPlugin(info)
   , mActive(false)
{
   /* Do nothing. */ ;
}

// NOTE: This is here in the .cpp file to ensure that vrkit::mode::Component
// is compiled properly.  Should this code ever become part of a library, it
// is important to ensure that polymorphic types are compiled into the correct
// place.
Component::~Component()
{
   /* Do nothing. */ ;
}

bool Component::validatePluginLib(vpr::LibraryPtr pluginLib)
{
   vprASSERT(pluginLib->isLoaded() && "Plug-in library is not loaded");

   const std::string get_version_func("getComponentInterfaceVersion");

   void* version_symbol = pluginLib->findSymbol(get_version_func);

   if ( version_symbol == NULL )
   {
      std::ostringstream msg_stream;
      msg_stream << "Plug-in '" << pluginLib->getName()
                 << "' has no entry point function named "
                 << get_version_func;
      throw PluginInterfaceException(msg_stream.str(), VRKIT_LOCATION);
   }
   else
   {
      void (*version_func)(vpr::Uint32&, vpr::Uint32&);
      version_func = (void (*)(vpr::Uint32&, vpr::Uint32&)) version_symbol;

      vpr::Uint32 major_ver;
      vpr::Uint32 minor_ver;
      (*version_func)(major_ver, minor_ver);

      if ( major_ver != VRKIT_MODE_COMPONENT_API_MAJOR )
      {
         std::ostringstream msg_stream;
         msg_stream << "Interface version mismatch: run-time does not match "
                    << "compile-time plug-in setting ("
                    << VRKIT_MODE_COMPONENT_API_MAJOR << "."
                    << VRKIT_MODE_COMPONENT_API_MINOR << " != "
                    << major_ver << "." << minor_ver << ")";
         throw PluginInterfaceException(msg_stream.str(), VRKIT_LOCATION);
      }
   }

   return true;
}

bool Component::activate(vrkit::ViewerPtr viewer)
{
   try
   {
      doActivate(viewer);
      mActive = true;
   }
   catch (std::exception& ex)
   {
      std::cerr << "Failed to activate mode component '" << getDescription()
                << "':\n" << ex.what() << std::endl;
   }

   return mActive;
}

bool Component::deactivate(ViewerPtr viewer)
{
   vprASSERT(mActive && "Cannot deactivate an inactive component");

   try
   {
      doDeactivate(viewer);
      mActive = false;
   }
   catch (std::exception& ex)
   {
      std::cerr << "Failed to deactivate mode component '" << getDescription()
                << "':\n" << ex.what() << std::endl;
   }

   return ! mActive;
}

void Component::exit(ViewerPtr viewer)
{
   if ( mActive )
   {
      if ( ! deactivate(viewer) )
      {
         std::cerr << "WARNING: Component deactivation failed before exiting."
                   << std::endl;
      }
   }

   doExit(viewer);
}

}

}
