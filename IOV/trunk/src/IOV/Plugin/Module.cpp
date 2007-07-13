// Copyright (C) Infiscape Corporation 2005-2007

#include <cstdlib>
#include <sstream>

#include <IOV/Util/Exceptions.h>
#include <IOV/Plugin/Module.h>


namespace inf
{

namespace plugin
{

Module::Module(vpr::LibraryPtr module)
   : mModule(module)
{
   /* Do nothing. */ ;
}

bool Module::hasSymbol(const std::string& name) const
{
   return mModule->findSymbol(name) != NULL;
}

void* Module::getSymbol(const std::string& name) const
{
   void* symbol = mModule->findSymbol(name);

   if ( NULL == symbol )
   {
      std::ostringstream msg_stream;
      msg_stream << "Plug-in module '" << mModule->getName()
                 << "' has no entry point function named " << name;
      throw inf::PluginInterfaceException(msg_stream.str(), IOV_LOCATION);
   }

   return symbol;
}

}

}
