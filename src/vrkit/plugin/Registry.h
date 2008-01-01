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

#ifndef _VRKIT_PLUGIN_REGISTRY_H_
#define _VRKIT_PLUGIN_REGISTRY_H_

#include <vrkit/Config.h>

#include <string>
#include <map>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/signal.hpp>

#include <vpr/DynLoad/Library.h>

#include <vrkit/AbstractPluginPtr.h>
#include <vrkit/signal/Proxy.h>
#include <vrkit/plugin/RegistryEntryPtr.h>
#include <vrkit/plugin/RegistryPtr.h>


namespace vrkit
{

namespace plugin
{

class Info;

/** \class Registry Registry.h vrkit/plugin/Registry.h
 *
 * The centralized container for plug-in creators. The creators are held by
 * vrkit::plugin::RegistryEntry objects, which will generally be handed as
 * instantiations of vrkit::plugin::TypedRegistryEntry<T> or
 * vrkit::plugin::TypedInitRegistryEntry<T,R>.
 *
 * @since 0.36
 */
class VRKIT_CLASS_API Registry
   : public boost::enable_shared_from_this<Registry>
   , private boost::noncopyable
{
private:
   Registry();

public:
   static RegistryPtr create()
   {
      return RegistryPtr(new Registry());
   }

   ~Registry();

   /**
    * Performs post-creation initialization steps.
    *
    * @return This object is returned as a shared pointer.
    */
   RegistryPtr init();

   /** @name Signal Accessors for Slot Connections */
   //@{
   /** Plug-in instantiation signal. */
   typedef boost::signal<void (AbstractPluginPtr)> instance_signal_t;

   typedef boost::signal<void (const plugin::Info&, vpr::LibraryPtr)>
      register_signal_t;
   typedef boost::signal<void (vpr::LibraryPtr)> removal_signal_t;

   signal::Proxy<instance_signal_t> pluginInstantiated()
   {
      return signal::Proxy<instance_signal_t>(mPluginInstantiated);
   }

   signal::Proxy<register_signal_t> moduleRegistered()
   {
      return signal::Proxy<register_signal_t>(mModuleRegistered);
   }

   signal::Proxy<removal_signal_t> moduleRemoved()
   {
      return signal::Proxy<removal_signal_t>(mModuleRemoved);
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
    *              vrkit::Plugin::Info object that is used to perform the
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
   AbstractPluginPtr makeInstance(const std::string& pluginTypeID,
                                  std::vector<AbstractPluginPtr>& deps);

   AbstractPluginPtr makeNamedInstance(const std::string& pluginTypeID,
                                       const std::string& instanceName,
                                       std::vector<AbstractPluginPtr>& deps);
   //@}

   /** @name Instance Acquisition Interface */
   //@{
   AbstractPluginPtr getInstanceByInfo(const plugin::Info& info) const;

   AbstractPluginPtr getInstanceByType(const std::string& pluginTypeID) const;

   AbstractPluginPtr getInstanceByName(const std::string& name) const;
   //@}

private:
   void registerInstantiatedPlugin(const plugin::Info& pluginInfo,
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

}


#endif /* _VRKIT_PLUGIN_REGISTRY_H_ */
