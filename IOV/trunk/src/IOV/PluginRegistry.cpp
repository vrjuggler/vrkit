// Copyright (C) Infiscape Corporation 2005-2007

#include <sstream>
#include <stdexcept>
#include <queue>
#include <vector>
#include <utility>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/filesystem/path.hpp>

#include <vpr/vpr.h>
#include <vpr/vprParam.h>
#include <vpr/DynLoad/LibraryFinder.h>
#include <vpr/Util/Assert.h>

#if __VPR_version >= 1001005
#  include <vpr/IO/IOException.h>
#endif
 
#include <IOV/PluginRegistry.h>


namespace fs = boost::filesystem;

namespace inf
{

PluginRegistry::PluginRegistry()
{
   /* Do nothing. */ ;
}

PluginRegistry::~PluginRegistry()
{
   /* Do nothing. */ ;
}

inf::RegistryEntryPtr PluginRegistry::preCreate(const std::string& name)
{
   RegistryEntryPtr entry = findEntry(name);

   if ( ! entry )
   {
      std::ostringstream msg_stream;
      msg_stream << "No entry for " << name << " found";
      throw std::invalid_argument(msg_stream.str());
   }

   satisfyDeps(entry);

   return entry;
}

void PluginRegistry::postCreate(RegistryEntryPtr entry,
                                AbstractPluginPtr plugin)
{
   registerInstantiatedPlugin(entry->getInfo(), plugin);
}

void PluginRegistry::
registerInstantiatedPlugin(const inf::plugin::Info& pluginInfo,
                           AbstractPluginPtr p)
{
   mInstantiated.insert(std::make_pair(pluginInfo.getFullName(), p));
   mPluginInstantiated(p);
}

void PluginRegistry::addEntry(RegistryEntryPtr entry)
{
   mRegistry[entry->getInfo().getFullName()] = entry;
   mModuleRegistered(entry->getInfo(), entry->getModule());
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

struct is_version_less
{
   bool operator()(RegistryEntryPtr lhs, RegistryEntryPtr rhs)
   {
      return lhs->getInfo().getFullName() < rhs->getInfo().getFullName();
   }
};

RegistryEntryPtr
PluginRegistry::findNewestVersionEntry(const std::string& moduleName) const
{
   vprASSERT(moduleName.find(inf::plugin::Info::getSeparator()) == std::string::npos);

   std::priority_queue<
      RegistryEntryPtr, std::vector<RegistryEntryPtr>, is_version_less
   > queue;

   typedef registry_type::const_iterator iter_type;
   for ( iter_type i = mRegistry.begin(); i != mRegistry.end(); ++i )
   {
      if ( boost::algorithm::starts_with((*i).first, moduleName) )
      {
         queue.push((*i).second);
      }
   }

   return queue.top();
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
            msg_stream << "Missing dependency " << *d << " of "
                       << entry->getInfo().getFullName();
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
      }
      catch (inf::PluginDependencyException& ex)
      {
         std::ostringstream msg_stream;
         msg_stream << "Failed to satisfy dependencies of "
                    << dep->getInfo().getFullName() << ", a dependency of "
                    << entry->getInfo().getFullName() << std::endl
                    << ex.what();
         throw inf::PluginException(msg_stream.str(), IOV_LOCATION);
      }

      AbstractPluginPtr p = entry->createAsDependency();
      registerInstantiatedPlugin(entry->getInfo(), p);
      dependencies.pop();
   }
}

}
