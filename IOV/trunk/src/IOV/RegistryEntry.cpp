// Copyright (C) Infiscape Corporation 2005-2007

#include <sstream>

#include <IOV/AbstractPlugin.h>
#include <IOV/Util/Exceptions.h>
#include <IOV/Plugin/Module.h>
#include <IOV/RegistryEntry.h>


namespace inf
{

RegistryEntry::RegistryEntry(vpr::LibraryPtr module)
   : mModule(module)
   , mModuleInfo(getModuleInfo(module))
{
}

RegistryEntry::~RegistryEntry()
{
}

inf::PluginCreatorBase*
RegistryEntry::getCreatorFunc(vpr::LibraryPtr module,
                              const std::string& getCreatorFuncName)
   const
{
   inf::plugin::Module pm(module);
   return pm.getFunction<inf::PluginCreatorBase* ()>(getCreatorFuncName)();
}

inf::plugin::Info RegistryEntry::getModuleInfo(vpr::LibraryPtr module)
{
   inf::plugin::Module pm(module);
   return pm.getFunction<plugin::Info ()>(inf::AbstractPlugin::getInfoFuncName())();
}

}
