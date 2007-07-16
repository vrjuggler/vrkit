// Copyright (C) Infiscape Corporation 2005-2007

#include <sstream>

#include <vpr/Util/Assert.h>

#include <IOV/Plugin/Module.h>
#include <IOV/Plugin/Info.h>
#include <IOV/Util/Exceptions.h>
#include <IOV/AbstractPlugin.h>


namespace inf
{

AbstractPlugin::AbstractPlugin(const inf::plugin::Info& info)
   : mInfo(info)
{
   ;
}

AbstractPlugin::~AbstractPlugin()
{
   ;
}

void AbstractPlugin::basicValidation(const inf::plugin::Module& module)
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
      throw inf::PluginInterfaceException(msg_stream.str(), IOV_LOCATION);
   }
}

}
