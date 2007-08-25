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

#ifndef _INF_REGISTRY_ENTRY_H_
#define _INF_REGISTRY_ENTRY_H_

#include <IOV/Config.h>

#include <string>
#include <boost/noncopyable.hpp>

#include <vpr/DynLoad/Library.h>

#include <IOV/AbstractPluginPtr.h>
#include <IOV/Plugin/Info.h>
#include <IOV/RegistryEntryPtr.h>


namespace inf
{

class PluginCreatorBase;

/**
 * @since 0.36
 */
class IOV_CLASS_API RegistryEntry : private boost::noncopyable
{
protected:
   RegistryEntry(vpr::LibraryPtr module);

public:
   virtual ~RegistryEntry();

   const inf::plugin::Info& getInfo() const
   {
      return mModuleInfo;
   }

   vpr::LibraryPtr getModule() const
   {
      return mModule;
   }

   virtual inf::AbstractPluginPtr create() = 0;

protected:
   inf::PluginCreatorBase* getCreatorFunc(
      vpr::LibraryPtr module,
      const std::string& getCreatorFuncName
   ) const;

private:
   inf::plugin::Info getModuleInfo(vpr::LibraryPtr module);

   vpr::LibraryPtr   mModule;
   inf::plugin::Info mModuleInfo;
};

}


#endif /* _INF_REGISTRY_ENTRY_H_ */
