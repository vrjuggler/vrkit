// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_TYPED_REGISTRY_ENTRY_H_
#define _INF_TYPED_REGISTRY_ENTRY_H_

#include <IOV/Config.h>

#include <sstream>

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/cast.hpp>

#include <vpr/DynLoad/Library.h>

#include <IOV/PluginCreator.h>
#include <IOV/RegistryEntry.h>
#include <IOV/Util/Exceptions.h>


namespace inf
{

/**
 * @since 0.36
 */
template<typename T>
class TypedRegistryEntry
   : public RegistryEntry
{
public:
   typedef boost::shared_ptr<TypedRegistryEntry> ptr_type;

   /** @name Types for Plug-ins */
   //@{
   typedef boost::function<bool (vpr::LibraryPtr)> validator_func_type;
   typedef typename T::ptr_type plugin_ptr_type;
   typedef boost::function<plugin_ptr_type (plugin_ptr_type)> init_func_type;
   //@}

private:
   TypedRegistryEntry(vpr::LibraryPtr module, validator_func_type validator,
                      init_func_type initFunc)
      : RegistryEntry(module)
      , mCreator(NULL)
      , mInitFunc(initFunc)
   {
      if ( validator(module) )
      {
         // RegistryEntry::getCreatorFunc() will throw an exception if the
         // creator function could not be found.
         mCreator =
            boost::polymorphic_downcast<inf::PluginCreator<T>*>(
               getCreatorFunc(module, T::getCreatorFuncName())
            );

         // At this point, we are good to go, so we can finish initializing
         // this object.
         mModule   = module;
         mInitFunc = initFunc;
      }
      else
      {
         std::ostringstream msg_stream;
         msg_stream << "Module '" << module->getName() << "' is invalid!";
         throw inf::PluginException(msg_stream.str(), IOV_LOCATION);
      }
   }

public:
   static RegistryEntryPtr create(vpr::LibraryPtr module,
                                  validator_func_type validator,
                                  init_func_type initFunc = NULL)
   {
      return RegistryEntryPtr(new TypedRegistryEntry(module, validator,
                                                     initFunc));
   }

   virtual ~TypedRegistryEntry()
   {
      /* Do nothing. */ ;
   }

   virtual AbstractPluginPtr createAsDependency()
   {
      return createPlugin();
   }

   plugin_ptr_type createPlugin()
   {
      plugin_ptr_type plugin_inst = mCreator->createPlugin();

      if ( ! mInitFunc.empty() )
      {
         plugin_inst = mInitFunc(plugin_inst);
      }

      return plugin_inst;
   }

   init_func_type getInitFunc() const
   {
      return mInitFunc;
   }

private:
   inf::PluginCreator<T>* mCreator;
   init_func_type         mInitFunc;
};

}


#endif /* _INF_TYPED_REGISTRY_ENTRY_H_ */
