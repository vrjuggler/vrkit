// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_PLUGIN_REGISTRY_H_
#define _INF_PLUGIN_REGISTRY_H_

#include <IOV/Config.h>

#include <string>
#include <map>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/signal.hpp>

#include <vpr/DynLoad/Library.h>

#include <IOV/AbstractPluginPtr.h>
#include <IOV/RegistryEntryPtr.h>
#include <IOV/Util/SignalProxy.h>
#include <IOV/PluginRegistryPtr.h>


namespace inf
{

namespace plugin
{

class Info;

}

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

   /**
    * Performs post-creation initialization steps.
    *
    * @return This object is returned as a shared pointer.
    */
   PluginRegistryPtr init();

   /** @name Signal Accessors for Slot Connections */
   //@{
   /** Plug-in instantiation signal. */
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

   /**
    * Adds the given registry entry to this registry if the associated plug-in
    * has not already been registered. If \p entry is for a known plug-in
    * (determined using its full name), then the duplicate entry is ignored.
    *
    * @post The plug-in referred to by \p entry is a member of this plug-in
    *       registry.
    *
    * @param entry The registry entry to add. This contains an
    *              inf::Plugin::Info object that is used to perform the
    *              registration process. It is this object that is queried to
    *              get the full name of the plug-in to determine whether the
    *              plug-in associated with the given entry is already
    *              registered.
    *
    * @return \c true is returned if \p entry is added to the registry;
    *         \c false otherwise.
    */
   bool addEntry(RegistryEntryPtr entry);

   /** @name Plug-In Instance Creation Interface */
   //@{
   inf::AbstractPluginPtr makeInstance(
      const std::string& pluginTypeID,
      std::vector<inf::AbstractPluginPtr>& deps
   );

   inf::AbstractPluginPtr makeNamedInstance(
      const std::string& pluginTypeID, const std::string& instanceName,
      std::vector<inf::AbstractPluginPtr>& deps
   );
   //@}

   /** @name Instance Acquisition Interface */
   //@{
   inf::AbstractPluginPtr getInstanceByInfo(const inf::plugin::Info& info)
      const;

   inf::AbstractPluginPtr getInstanceByType(const std::string& pluginTypeID)
      const;

   inf::AbstractPluginPtr getInstanceByName(const std::string& name) const;
   //@}

private:
   void registerInstantiatedPlugin(const inf::plugin::Info& pluginInfo,
                                   AbstractPluginPtr plugin);

   RegistryEntryPtr findEntry(const std::string& moduleName) const;

   RegistryEntryPtr findNewestVersionEntry(const std::string& moduleName)
      const;

   AbstractPluginPtr findNewestVersionInstance(const std::string& moduleName)
      const;

   void satisfyDeps(RegistryEntryPtr entry);

   /** @name The Registry */
   //@{
   typedef std::map<std::string, RegistryEntryPtr> registry_type;
   registry_type mRegistry;

   std::map<std::string, AbstractPluginPtr> mNamedInstances;
   std::multimap<std::string, AbstractPluginPtr> mInstantiated;
   std::vector<std::string> mInProgress;
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
