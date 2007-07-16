// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_TYPED_INIT_REGISTRY_ENTRY_H_
#define _INF_TYPED_INIT_REGISTRY_ENTRY_H_

#include <IOV/Config.h>


#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

#include <vpr/DynLoad/Library.h>

#include <IOV/TypedRegistryEntry.h>


namespace inf
{

/**
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


#endif /* _INF_TYPED_REGISTRY_ENTRY_H_ */
