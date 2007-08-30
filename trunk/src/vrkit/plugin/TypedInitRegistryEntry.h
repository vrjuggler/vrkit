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

#ifndef _VRKIT_PLUGIN_TYPED_INIT_REGISTRY_ENTRY_H_
#define _VRKIT_PLUGIN_TYPED_INIT_REGISTRY_ENTRY_H_

#include <vrkit/Config.h>

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

#include <vpr/DynLoad/Library.h>

#include <vrkit/plugin/TypedRegistryEntry.h>


namespace vrkit
{

namespace plugin
{

/**
 * @note This class was moved into the vrkit::plugin namespace in version 0.47.
 *
 * @since 0.36
 */
template<typename T, typename InitReturnType = typename T::ptr_type>
class TypedInitRegistryEntry
   : public TypedRegistryEntry<T>
{
public:
   typedef boost::shared_ptr<TypedInitRegistryEntry> ptr_type;
   typedef TypedRegistryEntry<T>                     base_type;
   typedef typename base_type::validator_func_type   validator_func_type;

   /** @name Types for Plug-ins */
   //@{
   /** Function object for storing the plug-in initialization function. */
   typedef boost::function<
      InitReturnType (typename T::ptr_type)
   > init_func_type;
   //@}

private:
   TypedInitRegistryEntry(vpr::LibraryPtr module,
                          validator_func_type validator,
                          init_func_type initFunc)
      : base_type(module, validator)
      , mInitFunc(initFunc)
   {
      /* Do nothing. */
   }

public:
   static RegistryEntryPtr create(vpr::LibraryPtr module,
                                  validator_func_type validator,
                                  init_func_type initFunc)
   {
      return RegistryEntryPtr(new TypedInitRegistryEntry(module, validator,
                                                         initFunc));
   }

   virtual ~TypedInitRegistryEntry()
   {
      /* Do nothing. */ ;
   }

private:
   virtual typename base_type::plugin_ptr_type doCreate()
   {
      typename base_type::plugin_ptr_type plugin_inst = base_type::doCreate();
      mInitFunc(plugin_inst);
      return plugin_inst;
   }

   init_func_type mInitFunc;
};

}

}


#endif /* _VRKIT_PLUGIN_TYPED_INIT_REGISTRY_ENTRY_H_ */
