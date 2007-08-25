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
   typedef const plugin::Info* sig_type();
   return *pm.getFunction<sig_type>(inf::AbstractPlugin::getInfoFuncName())();
}

}
