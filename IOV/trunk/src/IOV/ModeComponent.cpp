// Copyright (C) Infiscape Corporation 2005-2007

#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <exception>

#include <vpr/vpr.h>
#include <vpr/Util/Assert.h>

#include <IOV/Util/Exceptions.h>
#include <IOV/ModeComponent.h>


namespace inf
{

bool ModeComponent::validatePluginLib(vpr::LibraryPtr pluginLib)
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
      throw inf::PluginInterfaceException(msg_stream.str(), IOV_LOCATION);
   }
   else
   {
      void (*version_func)(vpr::Uint32&, vpr::Uint32&);
      version_func = (void (*)(vpr::Uint32&, vpr::Uint32&)) version_symbol;

      vpr::Uint32 major_ver;
      vpr::Uint32 minor_ver;
      (*version_func)(major_ver, minor_ver);

      if ( major_ver != INF_MODE_COMPONENT_API_MAJOR )
      {
         std::ostringstream msg_stream;
         msg_stream << "Interface version mismatch: run-time does not match "
                    << "compile-time plug-in setting ("
                    << INF_MODE_COMPONENT_API_MAJOR << "."
                    << INF_MODE_COMPONENT_API_MINOR << " != "
                    << major_ver << "." << minor_ver << ")";
         throw inf::PluginInterfaceException(msg_stream.str(), IOV_LOCATION);
      }
   }

   return true;
}

// NOTE: This is here in the .cpp file to ensure that inf::ModeComponent is
// compiled properly.  Should this code ever become part of a library, it is
// important to ensure that polymorphic types are compiled into the correct
// place.
ModeComponent::~ModeComponent()
{
   /* Do nothing. */ ;
}

bool ModeComponent::activate(inf::ViewerPtr viewer)
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

bool ModeComponent::deactivate(inf::ViewerPtr viewer)
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

void ModeComponent::exit(inf::ViewerPtr viewer)
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

// NOTE: This is here in the .cpp file to ensure that inf::ModeComponent is
// compiled properly.  Should this code ever become part of a library, it is
// important to ensure that polymorphic types are compiled into the correct
// place.
ModeComponent::ModeComponent(const inf::plugin::Info& info)
   : AbstractPlugin(info)
   , mActive(false)
{
   /* Do nothing. */ ;
}

}
