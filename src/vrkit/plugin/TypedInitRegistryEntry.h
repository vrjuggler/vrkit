// vrkit is (C) Copyright 2005-2011
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

/** \class TypedInitRegistryEntry TypedInitRegistryEntry.h vrkit/plugin/TypedInitRegistryEntry.h
 *
 * A plug-in registry entry type for vrrkit::plugin::Registry where the
 * created instance, a subclass of vrkit::AbstractPlugin, has a post-creation
 * initialization procedure. The type of plug-in instance that is created by
 * the create() method is not strictly important because that type may not be
 * known at the time that vrkit and other users of this type are compiled.
 *
 * The tricky part here is knowing the return type of the initialization
 * procedure. The vrkit idiom is to have an init() method that returns a
 * shared pointer to the object being initialized (by calling
 * shared_from_this() or equivalent). Some initialzation procedures may not
 * have a return type. Ultimately, the return type only matters for the
 * purposes of C++ strong typing because, as can be seen in doCreate(), the
 * value, if any, returned by the initialization procedure is ignored.
 *
 * @see vrkit::plugin::Creator
 *
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
   /**
    * Constructor for the case when the creator function must be queried at
    * run time from the given plug-in module object.
    *
    * @param module    The dynamically loaded library from which the creator
    *                  will be retrieved.
    * @param validator A validator used to ensure that \p module provides
    *                  what is necessary to create instances of the plug-in.
    * @param initFunc  The post-creation initialization callable. This takes
    *                  as its only argument the plug-in instance that is
    *                  created in doCreate(). Because this uses Boost.Function,
    *                  it can actually be anything that can be represented
    *                  using boost::bind().
    */
   TypedInitRegistryEntry(vpr::LibraryPtr module,
                          validator_func_type validator,
                          init_func_type initFunc)
      : base_type(module, validator)
      , mInitFunc(initFunc)
   {
      /* Do nothing. */
   }

   /**
    * Constuctor for the case of a creator that is known statically at
    * compile time.
    *
    * @param creator  The type-specific creator of the plug-in objects. This
    *                 must refer to an object and a type \p T known at the
    *                 time that this code is compiled.
    * @param initFunc The post-creation initialization callable. This takes as
    *                 its only argument the plug-in instance that is created
    *                 in doCreate(). Because this uses Boost.Function, it can
    *                 actually be anything that can be represented using
    *                 boost::bind().
    *
    * @since 0.51.0
    */
   TypedInitRegistryEntry(Creator<T>* creator, init_func_type initFunc)
      : base_type(creator)
      , mInitFunc(initFunc)
   {
      /* Do nothing. */
   }

public:
   /**
    * Creates a registry entry for vrrkit::plugin::Registry where the creator
    * function must be looked up at run time from a dynamically loaded
    * library.
    *
    * @param module    The dynamically loaded library from which the creator
    *                  will be retrieved.
    * @param validator A validator used to ensure that \p module provides
    *                  what is necessary to create instances of the plug-in.
    * @param initFunc  The post-creation initialization callable. This takes
    *                  as its only argument the plug-in instance that is
    *                  created in doCreate(). Because this uses Boost.Function,
    *                  it can actually be anything that can be represented
    *                  using boost::bind().
    */
   static RegistryEntryPtr create(vpr::LibraryPtr module,
                                  validator_func_type validator,
                                  init_func_type initFunc)
   {
      return RegistryEntryPtr(new TypedInitRegistryEntry(module, validator,
                                                         initFunc));
   }

   /**
    * Creates a registry entry for vrkit::plugin::Registry where the creator
    * function is compiled into the code statically rather than being loaded
    * dynamically from a plug-in module. This is not appropriate for cases
    * when the creator function must be looked up at run time from a
    * dynamically loaded library. The object returned by
    * vrkit::plugin::RegistryEntry::getModule() will be a null shared pointer.
    *
    * @param creator  The type-specific creator of the plug-in objects. This
    *                 must refer to an object and a type \p T known at the
    *                 time that this code is compiled.
    * @param initFunc The post-creation initialization callable. This takes as
    *                 its only argument the plug-in instance that is created
    *                 in doCreate(). Because this uses Boost.Function, it can
    *                 actually be anything that can be represented using
    *                 boost::bind().
    *
    * @since 0.51.0
    */
   static RegistryEntryPtr create(Creator<T>* creator,
                                  init_func_type initFunc)
   {
      return RegistryEntryPtr(new TypedInitRegistryEntry(creator, initFunc));
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
