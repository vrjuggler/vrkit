// Copyright (C) Infiscape Corporation 2005-2006

#include <cstdlib>
#include <string>
#include <sstream>

#include <vpr/vpr.h>
#include <vpr/Util/Assert.h>

#include <IOV/Util/Exceptions.h>
#include <IOV/Plugin.h>


namespace inf
{

bool Plugin::validatePluginLib(vpr::LibraryPtr pluginLib)
{
   vprASSERT(pluginLib->isLoaded() && "Plug-in library is not loaded");

   const std::string get_version_func("getPluginInterfaceVersion");

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

      if ( major_ver != INF_PLUGIN_API_MAJOR )
      {
         std::ostringstream msg_stream;
         msg_stream << "Interface version mismatch: run-time does not match "
                    << "compile-time plug-in setting ("
                    << INF_PLUGIN_API_MAJOR << "." << INF_PLUGIN_API_MINOR
                    << " != " << major_ver << "." << minor_ver << ")";
         throw inf::PluginInterfaceException(msg_stream.str(),
                                             IOV_LOCATION);
      }
   }

   return true;
}

// NOTE: This is here in the .cpp file to ensure that inf::Plugin is
// compiled properly.  Should this code ever become part of a library, it is
// important to ensure that polymorphic types are compiled into the correct
// place.
Plugin::~Plugin()
{
   /* Do nothing. */ ;
}

// NOTE: This is here in the .cpp file to ensure that inf::Plugin is
// compiled properly.  Should this code ever become part of a library, it is
// important to ensure that polymorphic types are compiled into the correct
// place.
Plugin::Plugin()
   : mIsFocused(false)
{
   /* Do nothing. */ ;
}

}
