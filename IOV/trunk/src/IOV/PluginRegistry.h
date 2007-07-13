// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_PLUGIN_REGISTRY_H_
#define _INF_PLUGIN_REGISTRY_H_

#include <IOV/Config.h>

#include <sstream>
#include <string>
#include <map>
#include <stdexcept>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/signal.hpp>

#include <vpr/DynLoad/Library.h>

#include <IOV/AbstractPluginPtr.h>
#include <IOV/PluginCreator.h>
#include <IOV/TypedRegistryEntry.h>
#include <IOV/Plugin/Info.h>
#include <IOV/Util/SignalProxy.h>
#include <IOV/PluginRegistryPtr.h>


namespace inf
{

/**
 *
 * @since 0.36
 */
class IOV_CLASS_API PluginRegistry
   : public boost::enable_shared_from_this<PluginRegistry>
   , private boost::noncopyable
{
private:
   PluginRegistry();

public:
   static PluginRegistryPtr create()
   {
      return PluginRegistryPtr(new PluginRegistry());
   }

   ~PluginRegistry();

   /** @name Signal Accessors for Slot Connections */
   //@{
   typedef boost::signal<void (inf::AbstractPluginPtr)> instance_signal_t;
   typedef boost::signal<void (const inf::plugin::Info&, vpr::LibraryPtr)>
      register_signal_t;
   typedef boost::signal<void (vpr::LibraryPtr)> removal_signal_t;

   SignalProxy<instance_signal_t> pluginInstantiated()
   {
      return SignalProxy<instance_signal_t>(mPluginInstantiated);
   }

   SignalProxy<register_signal_t> moduleRegistered()
   {
      return SignalProxy<register_signal_t>(mModuleRegistered);
   }

   SignalProxy<removal_signal_t> moduleRemoved()
   {
      return SignalProxy<removal_signal_t>(mModuleRemoved);
   }
   //@}

   template<typename T>
   void registerModule(
      vpr::LibraryPtr pluginLib,
      typename TypedRegistryEntry<T>::validator_func_type validatorFunc,
      typename TypedRegistryEntry<T>::init_func_type initFunc = NULL
   )
   {
      addEntry(TypedRegistryEntry<T>::create(pluginLib, validatorFunc,
                                             initFunc));
   }

   template<typename T>
   typename T::ptr_type create(const std::string& name)
   {
      typename TypedRegistryEntry<T>::ptr_type typed_entry =
         boost::dynamic_pointer_cast< TypedRegistryEntry<T> >(preCreate(name));

      if ( ! typed_entry )
      {
         throw std::bad_cast();
      }

      satisfyDeps(typed_entry);

      typename T::ptr_type plugin = typed_entry->createPlugin();
      postCreate(typed_entry, plugin);

      return plugin;
   }

private:
   RegistryEntryPtr preCreate(const std::string& name);

   void postCreate(RegistryEntryPtr entry, AbstractPluginPtr plugin);

   void registerInstantiatedPlugin(const inf::plugin::Info& pluginInfo,
                                   AbstractPluginPtr plugin);

   void addEntry(RegistryEntryPtr entry);

   RegistryEntryPtr findEntry(const std::string& moduleName) const;

   RegistryEntryPtr findNewestVersionEntry(const std::string& moduleName)
      const;

   void satisfyDeps(RegistryEntryPtr entry);

   /** @name The Registry */
   //@{
   typedef std::map<std::string, RegistryEntryPtr> registry_type;
   registry_type mRegistry;

   std::multimap<std::string, AbstractPluginPtr> mInstantiated;
   //@}

   /** @name Signals */
   //@{
   instance_signal_t mPluginInstantiated;
   register_signal_t mModuleRegistered;
   removal_signal_t  mModuleRemoved;
   //@}
};

}


#endif /* _INF_PLUGIN_REGISTRY_H_ */
