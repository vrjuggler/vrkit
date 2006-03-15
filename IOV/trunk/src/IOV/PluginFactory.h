// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_PLUGIN_FACTORY_H_
#define _INF_PLUGIN_FACTORY_H_

#include <IOV/Config.h>

#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>

#include <vpr/vpr.h>
#include <vpr/DynLoad/Library.h>

#include <IOV/Util/Exceptions.h>
#include <IOV/PluginFactoryBase.h>
#include <IOV/PluginCreator.h>


namespace inf
{

/** Central factory for IOV plugins.
 * Takes care of scanning, finding, and returning plugin libraries.
 */
template<typename PLUGIN_TYPE>
class PluginFactory
   : public inf::PluginFactoryBase
   , public boost::enable_shared_from_this<PluginFactory<PLUGIN_TYPE> >
{
public:
   typedef boost::shared_ptr< PluginFactory<PLUGIN_TYPE> > ptr_t;
   typedef inf::PluginCreator<PLUGIN_TYPE> plugin_creator_t;
   typedef boost::shared_ptr<PLUGIN_TYPE> plugin_ptr_t;

   static ptr_t create()
   {
      return ptr_t(new PluginFactory<PLUGIN_TYPE>());
   }

   virtual ~PluginFactory()
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
    * @see getPluginLibrary
    * @see getPluginCreator
    */
   void init(const std::vector<std::string>& scanPath)
   {
      addScanPath(scanPath);
   }

   /**
    * Returns the plugin_creator_t instance retrieved from the plug-in library
    * associated with the given platform-agnostic plug-in name. If the creator
    * instance has not already been looked up, the lookup will occur as a
    * result of calling this method.
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
   plugin_creator_t* getPluginCreator(const std::string& name)
      throw (inf::NoSuchPluginException, inf::PluginLoadException,
             inf::PluginInterfaceException)
   {
      typename plugin_creator_map_t::iterator i = mPluginCreators.find(name);

      if ( mPluginCreators.end() == i )
      {
         this->registerCreatorFromName(name);

         i = mPluginCreators.find(name);
         if ( mPluginCreators.end() == i )
         {
            std::ostringstream msg_stream;
            msg_stream << "Plug-in '" << name << "' failed to load";
            throw PluginLoadException(msg_stream.str(), IOV_LOCATION);
         }
      }

      return (*i).second;
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
   void registerCreator(plugin_creator_t* creator, const std::string& name)
   {
      if ( mPluginCreators.find(name) != mPluginCreators.end() )
      {
         std::cerr << "WARNING: Tried to re-register a creator for plug-in "
                   << "type '" << name << "'!\n"
                   << "This registration will be ignored." << std::endl;
         return;
      }

      std::cout << "IOV: [PluginFactory::registerCreator()] "
                << "Registering creator for '" << name << "'" << std::endl;

      mPluginCreators[name] = creator;
   }

protected:
   PluginFactory()
      : PluginFactoryBase()
      , mGetCreatorFuncName(PLUGIN_TYPE::getCreatorFuncName())
      , mValidator(PLUGIN_TYPE::validatePluginLib)
   {
      /* Do nothing. */ ;
   }

   /**
    * Registers an instance of plugin_creator_t retrieved from the given
    * plug-in library using the given platform-agnostic plug-in name as the
    * registration key.
    *
    * @pre The plug-in library has been loaded.
    * @post If no exception is thrown, an entry will be added to
    *       mPluginCreators associating \c name with an instance of
    *       plugin_creator_t.
    *
    * @param pluginLib  The plug-in library that will be examined to get the
    *                   inf::PluginCreator<inf::Plugin> instance to reigster.
    * @param name       The platform-agnostic name of the plug-in that will
    *                   be used as the key for registering the
    *                   plugin_creator_t instance.
    *
    * @throw inf::PluginInterfaceException
    *           Thrown if the given plug-in library does not contain an
    *           entry point function for acquiring the plug-in's creator
    *           instance.
    */
   void registerCreatorFromLib(vpr::LibraryPtr pluginLib,
                               const std::string& name)
      throw (inf::PluginInterfaceException)
   {
      vprASSERT(pluginLib->isLoaded() && "Plug-in library is not loaded");

      if ( mValidator(pluginLib) )
      {
         void* creator_symbol = pluginLib->findSymbol(mGetCreatorFuncName);

         if ( creator_symbol != NULL )
         {
            plugin_creator_t* (*creator_func)();
            creator_func = (plugin_creator_t* (*)()) creator_symbol;
            plugin_creator_t* new_creator = (*creator_func)();
            registerCreator(new_creator, name);
         }
         else
         {
            std::ostringstream msg_stream;
            msg_stream << "Plug-in '" << pluginLib->getName()
                       << "' has no entry point function named "
                       << mGetCreatorFuncName;
            throw inf::PluginInterfaceException(msg_stream.str(),
                                                IOV_LOCATION);
         }
      }
   }

private:
   const std::string mGetCreatorFuncName;
   boost::function<bool (vpr::LibraryPtr)> mValidator;

   typedef std::map<std::string, plugin_creator_t*> plugin_creator_map_t;

   plugin_creator_map_t mPluginCreators;  /**< Map of plugin creators that have been registered. */
};

}


#endif
