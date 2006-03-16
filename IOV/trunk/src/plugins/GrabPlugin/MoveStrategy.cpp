#include <cstdlib>
#include <sstream>

#include <vpr/vpr.h>
#include <vpr/Util/Assert.h>

#include <IOV/Util/Exceptions.h>

#include "MoveStrategy.h"


namespace inf
{

bool MoveStrategy::validatePluginLib(vpr::LibraryPtr pluginLib)
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

      if ( major_ver != INF_MOVE_STRATEGY_PLUGIN_API_MAJOR )
      {
         std::ostringstream msg_stream;
         msg_stream << "Interface version mismatch: run-time does not match "
                    << "compile-time plug-in setting ("
                    << INF_MOVE_STRATEGY_PLUGIN_API_MAJOR << "."
                    << INF_MOVE_STRATEGY_PLUGIN_API_MINOR
                    << " != " << major_ver << "." << minor_ver << ")";
         throw inf::PluginInterfaceException(msg_stream.str(),
                                             IOV_LOCATION);
      }
   }

   return true;
}

MoveStrategy::~MoveStrategy()
{
   /* Do nothing. */ ;
}

MoveStrategy::MoveStrategy()
{
   /* Do nothing. */ ;
}

}
