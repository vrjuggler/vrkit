// Copyright (C) Infiscape Corporation 2005-2006

#include <iostream>
#include <sstream>

#include <boost/filesystem/path.hpp>

#include <vpr/vpr.h>
#include <vpr/vprParam.h>
#include <vpr/DynLoad/LibraryFinder.h>
#include <vpr/Util/Assert.h>

#if __VPR_version >= 1001005
#  include <vpr/IO/IOException.h>
#endif

#include <IOV/Plugin.h>
#include <IOV/PluginFactory.h>


namespace fs = boost::filesystem;

namespace inf
{

PluginFactoryPtr PluginFactory::init(const std::vector<std::string>& scanPath)
{
   addScanPath(scanPath);
   return shared_from_this();
}

void PluginFactory::addScanPath(const std::vector<std::string>& scanPath)
{
   // Determine the platform-specific file extension used for dynamically
   // loadable code.
#if defined(VPR_OS_Win32) || defined(VPR_OS_Windows)
   const std::string driver_ext("dll");
#elif defined(VPR_OS_Darwin)
   // NOTE: The file extension "bundle" may be appropriate in some cases.
   const std::string driver_ext("dylib");
#else
   const std::string driver_ext("so");
#endif

   // Determine the build-specific part of the dynamically loadable module
   // file name to be stripped off.  For a debug build, this would be "_d.so"
   // on most UNIX-based platforms.  For an optimized build, it would simply
   // be ".so" on the same platforms.
#if defined(_DEBUG)
   const std::string strip_str = std::string("_d.") + driver_ext;
#else
   const std::string strip_str = std::string(".") + driver_ext;
#endif

   std::vector<std::string>::const_iterator i;
   for ( i = scanPath.begin(); i != scanPath.end(); ++i )
   {
      try
      {
         vpr::LibraryFinder finder(*i, driver_ext);
         vpr::LibraryFinder::LibraryList libs = finder.getLibraries();

         for ( unsigned int j = 0; j < libs.size(); ++j )
         {
            // Construct the platform-agnostic plug-in name by getting the name
            // of the plug-in file (leading path information is removed) and
            // then stripping off strip_str.
            fs::path lib_path(libs[j]->getName(), fs::native);
            const std::string lib_name(lib_path.leaf());
            const std::string::size_type strip_pos = lib_name.find(strip_str);

            if ( strip_pos != std::string::npos )
            {
               const std::string plugin_name(lib_name.substr(0, strip_pos));

               // Register the vpr::LibraryPtr object using the
               // platform-agnostic name so that callers of getPluginLibrary()
               // and getPluginCreator() do not have to worry about
               // platform-specific naming issues.
               if(mPluginLibs.find(plugin_name) == mPluginLibs.end())
               {
                  mPluginLibs[plugin_name] = libs[j];
                  std::cout << "IOV: Found plugin: [" << plugin_name << "]" << std::endl;
               }
               else
               {
                  std::cout << "WARNING: IOV found a plugin that was already registered: " << plugin_name << std::endl;
               }
            }
            else
            {
               std::cout << "WARNING: Invalid plug-in library name encountered: '"
                         << libs[j]->getName() << "'" << std::endl;
            }
         }
      }
      catch (std::exception& ex)
      {
         std::cout << "Exception scanning plug-in path: " << (*i) << std::endl
                   << ex.what() << std::endl;
      }
   }
}

vpr::LibraryPtr PluginFactory::getPluginLibrary(const std::string& name) const
   throw(inf::NoSuchPluginException)
{
   std::map<std::string, vpr::LibraryPtr>::const_iterator lib =
      mPluginLibs.find(name);

   if ( lib != mPluginLibs.end() )
   {
      return (*lib).second;
   }
   else
   {
      std::stringstream msg_stream;
      msg_stream << "No plug-in named '" << name << "' exists";
      throw NoSuchPluginException(msg_stream.str(), IOV_LOCATION);
   }
}

void PluginFactory::registerCreator(inf::PluginCreatorBase* creator,
                                    const std::string& name)
{
   plugin_creator_map_t::iterator i = mPluginCreators.find(name);
   if(i != mPluginCreators.end())
   {
      std::cerr << "WARNING: Tried to re-register a creator for plugin type: " << name
                << ".  This registration will be ignored." << std::endl;
      return;
   }

   std::cout << "IOV: PluginFactory: Registering creator for: " << name << std::endl;

   mPluginCreators[name] = creator;
}

void PluginFactory::
registerCreatorFromName(const std::string& name,
                        const std::string& getCreatorFuncName,
                        boost::function<bool (vpr::LibraryPtr)> validator)
   throw (inf::PluginLoadException)
{
   // Get the vpr::LibraryPtr for the named plug-in.  This will throw an
   // exception if name is not a valid plug-in name.
   vpr::LibraryPtr plugin_lib = getPluginLibrary(name);

   // At this point, we know that the given name must be a valid plug-in
   // name.
   if ( plugin_lib->isLoaded() )
   {
      registerCreatorFromLib(plugin_lib, name, getCreatorFuncName, validator);
   }
   else
   {
#if __VPR_version >= 1001010
      try
      {
         plugin_lib->load();
         registerCreatorFromLib(plugin_lib, name, getCreatorFuncName,
                                validator);
      }
      catch (vpr::IOException& ex)
      {
         std::ostringstream msg_stream;
         msg_stream << "Plug-in '" << name << "' failed to load:\n"
                    << ex.getExtendedDescription();
         throw PluginLoadException(msg_stream.str(), IOV_LOCATION);
      }
#else
      if ( plugin_lib->load().success() )
      {
         registerCreatorFromLib(plugin_lib, name, getCreatorFuncName,
                                validator);
      }
      else
      {
         std::ostringstream msg_stream;
         msg_stream << "Plug-in '" << name << "' failed to load";
         throw PluginLoadException(msg_stream.str(), IOV_LOCATION);
      }
#endif
   }
}

void PluginFactory::
registerCreatorFromLib(vpr::LibraryPtr pluginLib, const std::string& name,
                       const std::string& getCreatorFuncName,
                       boost::function<bool (vpr::LibraryPtr)> validator)
   throw (inf::PluginInterfaceException)
{
   vprASSERT(pluginLib->isLoaded() && "Plug-in library is not loaded");

   if ( validator(pluginLib) )
   {
      void* creator_symbol = pluginLib->findSymbol(getCreatorFuncName);

      if ( creator_symbol != NULL )
      {
         inf::PluginCreatorBase* (*creator_func)();
         creator_func = (inf::PluginCreatorBase* (*)()) creator_symbol;
         inf::PluginCreatorBase* new_creator = (*creator_func)();
         registerCreator(new_creator, name);
      }
      else
      {
         std::ostringstream msg_stream;
         msg_stream << "Plug-in '" << pluginLib->getName()
                    << "' has no entry point function named "
                    << getCreatorFuncName;
         throw inf::PluginInterfaceException(msg_stream.str(),
                                             IOV_LOCATION);
      }
   }
}

}
