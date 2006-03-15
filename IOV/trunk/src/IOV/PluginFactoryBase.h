// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_PLUGIN_FACTORY_BASE_H_
#define _INF_PLUGIN_FACTORY_BASE_H_

#include <string>
#include <map>
#include <vector>

#include <vpr/DynLoad/Library.h>

#include <IOV/Util/Exceptions.h>


namespace inf
{

/**
 * @since 0.16.0
 */
class PluginFactoryBase
{
public:
   virtual ~PluginFactoryBase()
   {
   }

   /**
    * Extends the current list of plugin libraries by searching the given path
    * looking for additional plug-in libraries using platform- and
    * build-specific naming conventions.
    *
    * @see getPluginLibrary()
    */
   void addScanPath(const std::vector<std::string>& scanPath);

   /**
    * Returns the plug-in library associated with the given platform-agnostic
    * plug-in name.
    *
    * @param name       The platform-agnostic name of the desired plug-in.
    *
    * @throw NoSuchPluginException
    *           Thrown if the given plug-in name does not match any of the
    *           plug-ins discovered when this object was initialized.
    */
   vpr::LibraryPtr getPluginLibrary(const std::string& name) const
      throw (inf::NoSuchPluginException);

protected:
   PluginFactoryBase()
   {
   }

   void registerCreatorFromName(const std::string& name)
      throw (inf::PluginLoadException);

   /**
    * Registers an instance of a plug-in creator retrieved from the given
    * plug-in library using the given platform-agnostic plug-in name as the
    * registration key.
    *
    * @pre The plug-in library has been loaded.
    *
    * @param pluginLib The plug-in library that will be examined to get the
    *                  creator instance to reigster.
    * @param name      The platform-agnostic name of the plug-in that will be
    *                  used as the key for registering the creator instance.
    *
    * @throw inf::PluginInterfaceException
    *           Thrown if the given plug-in library does not contain an
    *           entry point function for acquiring the plug-in's creator
    *           instance.
    */
   virtual void registerCreatorFromLib(vpr::LibraryPtr pluginLib,
                                       const std::string& name)
      throw (inf::PluginInterfaceException) = 0;

   typedef std::map<std::string, vpr::LibraryPtr> plugin_libs_map_t;

   plugin_libs_map_t mPluginLibs; /**< Map of plug-in libs we know about. */
};

}


#endif /* _INF_PLUGIN_FACTORY_BASE_H_ */
