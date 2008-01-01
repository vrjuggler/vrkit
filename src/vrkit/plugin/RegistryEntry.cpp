// vrkit is (C) Copyright 2005-2008
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

#include <vrkit/AbstractPlugin.h>
#include <vrkit/plugin/Module.h>
#include <vrkit/plugin/RegistryEntry.h>


namespace vrkit
{

namespace plugin
{

RegistryEntry::RegistryEntry(vpr::LibraryPtr module)
   : mModule(module)
   , mModuleInfo(getModuleInfo(module))
{
   /* Do nothing. */ ;
}

RegistryEntry::~RegistryEntry()
{
}

CreatorBase*
RegistryEntry::getCreatorFunc(vpr::LibraryPtr module,
                              const std::string& getCreatorFuncName)
   const
{
   Module pm(module);
   return pm.getFunction<CreatorBase* ()>(getCreatorFuncName)();
}

Info RegistryEntry::getModuleInfo(vpr::LibraryPtr module)
{
   Module pm(module);
   typedef const Info* sig_type();
   return *pm.getFunction<sig_type>(AbstractPlugin::getInfoFuncName())();
}

}

}
