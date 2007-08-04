// Copyright (C) Infiscape Corporation 2005-2007

#include <sstream>
#include <stdexcept>
#include <queue>
#include <vector>
#include <iterator>
#include <utility>
#include <algorithm>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>

#include <vpr/vpr.h>
#include <vpr/vprParam.h>
#include <vpr/DynLoad/LibraryFinder.h>
#include <vpr/Util/Assert.h>

#include <IOV/AbstractPlugin.h>
#include <IOV/RegistryEntry.h>
#include <IOV/Util/Exceptions.h>
#include <IOV/PluginRegistry.h>


namespace
{

template<typename T>
struct is_version_less
{
   bool operator()(boost::shared_ptr<T> lhs, boost::shared_ptr<T> rhs)
   {
      return lhs->getInfo().getFullName() < rhs->getInfo().getFullName();
   }
};

}

namespace inf
{

PluginRegistry::PluginRegistry()
{
   /* Do nothing. */ ;
}

PluginRegistry::~PluginRegistry()
{
   mNamedInstances.clear();
   mInstantiated.clear();
   mRegistry.clear();
}

PluginRegistryPtr PluginRegistry::init()
{
   return shared_from_this();
}

inf::AbstractPluginPtr PluginRegistry::
makeInstance(const std::string& pluginTypeID,
             std::vector<inf::AbstractPluginPtr>& deps)
{
   RegistryEntryPtr entry = findEntry(pluginTypeID);

   if ( ! entry )
   {
      std::ostringstream msg_stream;
      msg_stream << "No registry entry for " << pluginTypeID << " was found";
      throw inf::PluginTypeNameException(msg_stream.str(), IOV_LOCATION);
   }

   satisfyDeps(entry);

   AbstractPluginPtr plugin = entry->create();
   registerInstantiatedPlugin(entry->getInfo(), plugin);

   return plugin;
}

inf::AbstractPluginPtr PluginRegistry::
makeNamedInstance(const std::string& pluginTypeID,
                  const std::string& instanceName,
                  std::vector<inf::AbstractPluginPtr>& deps)
{
   AbstractPluginPtr plugin = makeInstance(pluginTypeID, deps);
   mNamedInstances[instanceName] = plugin;
   return plugin;
}

inf::AbstractPluginPtr
PluginRegistry::getInstanceByInfo(const inf::plugin::Info& info) const
{
   return getInstanceByType(info.getFullName());
}

inf::AbstractPluginPtr
PluginRegistry::getInstanceByType(const std::string& pluginTypeID) const
{
   typedef std::multimap<std::string, AbstractPluginPtr> map_type;
   typedef map_type::const_iterator iter_type;

   AbstractPluginPtr plugin;
   iter_type i = mInstantiated.find(pluginTypeID);

   if ( i == mInstantiated.end() )
   {
      plugin = findNewestVersionInstance(pluginTypeID);

      if ( ! plugin )
      {
         std::ostringstream msg_stream;
         msg_stream << "No plug-ins of type " << pluginTypeID
                    << " have been instantiated";
         throw inf::PluginException(msg_stream.str(), IOV_LOCATION);
      }
   }
   else
   {
      plugin = (*i).second;
   }

   return plugin;
}

inf::AbstractPluginPtr
PluginRegistry::getInstanceByName(const std::string& name) const
{
   std::map<std::string, AbstractPluginPtr>::const_iterator i =
      mNamedInstances.find(name);

   if ( i == mNamedInstances.end() )
   {
      std::ostringstream msg_stream;
      msg_stream << "No plug-in named '" << name << "' exists";
      throw inf::PluginException(msg_stream.str(), IOV_LOCATION);
   }

   return (*i).second;
}

void PluginRegistry::
registerInstantiatedPlugin(const inf::plugin::Info& pluginInfo,
                           AbstractPluginPtr plugin)
{
   // Add the instantiated plug-in to the multi-map of plug-in objects.
   mInstantiated.insert(std::make_pair(pluginInfo.getFullName(), plugin));

   // Emit the instantiation signal.
   mPluginInstantiated(plugin);
}

bool PluginRegistry::addEntry(RegistryEntryPtr entry)
{
   bool result(false);
   const std::string& full_name(entry->getInfo().getFullName());

   if ( mRegistry.count(full_name) == 0 )
   {
      mRegistry[full_name] = entry;
      mModuleRegistered(entry->getInfo(), entry->getModule());
      result = true;
   }
#if defined(IOV_DEBUG)
   else
   {
      std::cerr << "NOTE: Igoring duplicate addition of " << full_name
                << std::endl;
   }
#endif

   return result;
}

RegistryEntryPtr PluginRegistry::findEntry(const std::string& moduleName) const
{
   registry_type::const_iterator i = mRegistry.find(moduleName);
   RegistryEntryPtr entry;

   if ( i != mRegistry.end() )
   {
      entry = (*i).second;
   }
   else
   {
      std::string::size_type split_pos =
         moduleName.find(inf::plugin::Info::getSeparator());
      entry = findNewestVersionEntry(moduleName.substr(0, split_pos));
   }

   return entry;
}

RegistryEntryPtr
PluginRegistry::findNewestVersionEntry(const std::string& moduleName) const
{
   vprASSERT(moduleName.find(inf::plugin::Info::getSeparator()) == std::string::npos);

   std::priority_queue<
      RegistryEntryPtr, std::vector<RegistryEntryPtr>,
      is_version_less<RegistryEntry>
   > queue;

   typedef registry_type::const_iterator iter_type;
   for ( iter_type i = mRegistry.begin(); i != mRegistry.end(); ++i )
   {
      if ( boost::algorithm::starts_with((*i).first, moduleName) )
      {
         queue.push((*i).second);
      }
   }

   RegistryEntryPtr entry;

   if ( ! queue.empty() )
   {
      entry = queue.top();
   }

   return entry;
}

AbstractPluginPtr
PluginRegistry::findNewestVersionInstance(const std::string& moduleName)
   const
{
   std::priority_queue<
      AbstractPluginPtr, std::vector<AbstractPluginPtr>,
      is_version_less<AbstractPlugin>
   > queue;

   typedef std::multimap<std::string, AbstractPluginPtr>::const_iterator
      iter_type;
   for ( iter_type i = mInstantiated.begin(); i != mInstantiated.end(); ++i )
   {
      if ( boost::algorithm::starts_with((*i).first, moduleName) )
      {
         queue.push((*i).second);
      }
   }

   AbstractPluginPtr plugin;

   if ( ! queue.empty() )
   {
      plugin = queue.top();
   }

   return plugin;
}

struct CompareDeps
{
   bool operator()(RegistryEntryPtr e1, RegistryEntryPtr e2)
   {
      const inf::plugin::Info& e1_info = e1->getInfo();
      const inf::plugin::Info& e2_info = e2->getInfo();

      if ( e1_info.getFullName() == e2_info.getFullName() )
      {
         return false;
      }

      bool result(false);

      // Plug-in 1 does not depend on Plug-in 2.
      if ( ! e1_info.dependsOn(e2_info) )
      {
         // Plug-in 2 does not depend on Plug-in 1.
         if ( ! e2_info.dependsOn(e1_info) )
         {
            result = true;
         }
         // Plug-in 2 depends on Plug-in 1.
      }
      // Plug-in 1 depends on Plug-in 2.
      else
      {
         result = true;
      }

      return result;
   }
};

void PluginRegistry::satisfyDeps(RegistryEntryPtr entry)
{
   const std::string& full_name(entry->getInfo().getFullName());
   std::vector<std::string>::iterator i = std::find(mInProgress.begin(),
                                                    mInProgress.end(),
                                                    full_name);

   // Circular dependency detected!
   // XXX: It would be nice to be able to detect this sort of problem before
   // beginning the recursive dependency satisfication.
   if ( i != mInProgress.end() )
   {
      std::ostringstream msg_stream;
      msg_stream << "Circular plug-in dependency detected:\n";
      std::copy(i, mInProgress.end(),
                std::ostream_iterator<std::string>(msg_stream, " -> "));
      msg_stream << full_name;
      throw inf::PluginDependencyException(msg_stream.str(), IOV_LOCATION);
   }

   mInProgress.push_back(full_name);

   const std::vector<std::string>& deps = entry->getInfo().getDependencies();

   std::priority_queue<
      RegistryEntryPtr, std::vector<RegistryEntryPtr>, CompareDeps
   > dependencies;

   typedef std::vector<std::string>::const_iterator iter_type;
   for ( iter_type d = deps.begin(); d != deps.end(); ++d )
   {
      if ( mInstantiated.count(*d) == 0 )
      {
         RegistryEntryPtr e = findEntry(*d);

         if ( e )
         {
            dependencies.push(e);
         }
         else
         {
            std::ostringstream msg_stream;
            msg_stream << "Missing dependency " << *d << " of " << full_name;
            throw inf::PluginDependencyException(msg_stream.str(),
                                                 IOV_LOCATION);
         }
      }
   }

   while ( ! dependencies.empty() )
   {
      RegistryEntryPtr dep = dependencies.top();

      try
      {
         satisfyDeps(dep);
         registerInstantiatedPlugin(entry->getInfo(), entry->create());
      }
      catch (inf::PluginDependencyException& ex)
      {
         std::ostringstream msg_stream;
         msg_stream << "Failed to satisfy dependencies of "
                    << dep->getInfo().getFullName() << ", a dependency of "
                    << full_name << std::endl << ex.what();
         throw inf::PluginException(msg_stream.str(), IOV_LOCATION);
      }

      dependencies.pop();
   }

   mInProgress.pop_back();
}

}
