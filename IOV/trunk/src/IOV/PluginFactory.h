// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_PLUGIN_FACTORY_H_
#define _INF_PLUGIN_FACTORY_H_

#include <IOV/Config.h>

#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/cast.hpp>

#include <vpr/vpr.h>
#include <vpr/DynLoad/Library.h>

#include <IOV/Util/Exceptions.h>
#include <IOV/PluginCreator.h>
#include <IOV/PluginFactoryPtr.h>


namespace inf
{

/** Central factory for IOV plugins.
 * Takes care of scanning, finding, and returning plugin libraries.
 */
class IOV_CLASS_API PluginFactory
   : public boost::enable_shared_from_this<PluginFactory>
{
public:
   static PluginFactoryPtr create()
   {
      return PluginFactoryPtr(new PluginFactory());
   }

   ~PluginFactory()
   {
      /* Do nothing. */ ;
   }

   /**
    * Scans the given search path looking for plug-in libraries using
    * platform- and build-specific naming conventions.  Any and all plug-in
    * libraries that are discovered are registered with this class using
    * the platform-agnostic name of the plug-in as the registration key.
    *
    * @param scanPath A list of directories to scan for plug-ins.
    *
    * @return This object is returned as a shared pointer.
    *
    * @see getPluginLibrary
    * @see getPluginCreator
    */
   PluginFactoryPtr init(const std::vector<std::string>& scanPath);

   /**
    * Extends the current list of plugin libraries by searching the given path
    * looking for additional plug-in libraries using platform- and
    * build-specific naming conventions.
    *
    * @param scanPath A list of directories to scan for plug-ins.
    *
    * @see getPluginLibrary
    * @see getPluginCreator
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

   /**
    * Returns the inf::PluginCreator<T>* instance retrieved from the plug-in
    * library associated with the given platform-agnostic plug-in name. If the
    * creator instance has not already been looked up, the lookup will occur
    * as a result of calling this method.
    *
    * @param name       The platform-agnostic name of the desired plug-in.
    *
    * @throw NoSuchPluginException
    *           Thrown if the given plug-in name does not match any of the
    *           plug-ins discovered when this object was initialized.
    * @throw PluginLoadException
    *           Thrown if the attempt to load the named plug-in failed for
    *           some reason.
    * @throw PluginInterfaceException
    *           Thrown if the plug-in library does not have the necessary
    *           entry point functions needed to access the plug-in or if the
    *           plug-in API version against which the plug-in was compiled
    *           does not match the run-time plug-in API version.
    */
   template<typename T>
   inf::PluginCreator<T>* getPluginCreator(const std::string& name)
      throw (inf::NoSuchPluginException, inf::PluginLoadException,
             inf::PluginInterfaceException)
   {
      typename plugin_creator_map_t::iterator i = mPluginCreators.find(name);

      if ( mPluginCreators.end() == i )
      {
         registerCreatorFromName(name, T::getCreatorFuncName(),
                                 T::validatePluginLib);

         i = mPluginCreators.find(name);
         if ( mPluginCreators.end() == i )
         {
            std::ostringstream msg_stream;
            msg_stream << "Plug-in '" << name << "' failed to load";
            throw PluginLoadException(msg_stream.str(), IOV_LOCATION);
         }
      }

      return boost::polymorphic_downcast<inf::PluginCreator<T>*>((*i).second);
   }

   /**
    * Manually register a creator with the plug-in factory.
    *
    * This method can be used to add creators that are not able to be found in
    * a dynamic library but are still available. This is sometimes the case
    * with custom plugins compiled directly into an application.
    *
    * @param creator    A valid creator
    * @param name       The name of the plugin type that is created by creator.
    */
   void registerCreator(inf::PluginCreatorBase* creator,
                        const std::string& name);

protected:
   PluginFactory()
   {
      /* Do nothing. */ ;
   }

private:
   void registerCreatorFromName(const std::string& name,
                                const std::string& getCreatorFuncName,
                                boost::function<bool (vpr::LibraryPtr)> validator)
      throw (inf::PluginLoadException);

   /**
    * Registers an instance of inf::PluginCreatorBase retrieved from the given
    * plug-in library using the given platform-agnostic plug-in name as the
    * registration key.
    *
    * @pre The plug-in library has been loaded.
    * @post If no exception is thrown, an entry will be added to
    *       mPluginCreators associating \c name with an instance of
    *       inf::PluginCreatorBase.
    *
    * @param pluginLib  The plug-in library that will be examined to get the
    *                   inf::PluginCreator<inf::Plugin> instance to reigster.
    * @param name       The platform-agnostic name of the plug-in that will
    *                   be used as the key for registering the
    *                   inf::PluginCreatorBase instance.
    *
    * @throw inf::PluginInterfaceException
    *           Thrown if the given plug-in library does not contain an
    *           entry point function for acquiring the plug-in's creator
    *           instance.
    */
   void registerCreatorFromLib(vpr::LibraryPtr pluginLib,
                               const std::string& name,
                               const std::string& getCreatorFuncName,
                               boost::function<bool (vpr::LibraryPtr)> validator)
      throw (inf::PluginInterfaceException);

   typedef std::map<std::string, vpr::LibraryPtr> plugin_libs_map_t;
   typedef std::map<std::string, inf::PluginCreatorBase*> plugin_creator_map_t;

   plugin_libs_map_t    mPluginLibs;      /**< Map of plugin libs we know about. */
   plugin_creator_map_t mPluginCreators;  /**< Map of plugin creators that have been registered. */
};

}


#endif
