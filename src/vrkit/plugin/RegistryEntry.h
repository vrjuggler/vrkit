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

#ifndef _VRKIT_PLUGIN_REGISTRY_ENTRY_H_
#define _VRKIT_PLUGIN_REGISTRY_ENTRY_H_

#include <vrkit/Config.h>

#include <string>
#include <boost/noncopyable.hpp>

#include <vpr/DynLoad/Library.h>

#include <vrkit/AbstractPluginPtr.h>
#include <vrkit/plugin/Info.h>
#include <vrkit/plugin/RegistryEntryPtr.h>


namespace vrkit
{

namespace plugin
{

class CreatorBase;

/** \class RegistryEntry RegistryEntry.h vrkit/plugin/RegistryEntry.h
 *
 * @note This class was moved into the vrkit::plugin namespace in version 0.47.
 *
 * @since 0.36
 */
class VRKIT_CLASS_API RegistryEntry : private boost::noncopyable
{
protected:
   RegistryEntry(vpr::LibraryPtr module);

public:
   virtual ~RegistryEntry();

   const Info& getInfo() const
   {
      return mModuleInfo;
   }

   vpr::LibraryPtr getModule() const
   {
      return mModule;
   }

   virtual AbstractPluginPtr create() = 0;

protected:
   CreatorBase* getCreatorFunc(
      vpr::LibraryPtr module, const std::string& getCreatorFuncName
   ) const;

private:
   Info getModuleInfo(vpr::LibraryPtr module);

   vpr::LibraryPtr mModule;
   Info            mModuleInfo;
};

}

}


#endif /* _VRKIT_PLUGIN_REGISTRY_ENTRY_H_ */
