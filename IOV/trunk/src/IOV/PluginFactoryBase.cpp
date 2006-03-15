// Copyright (C) Infiscape Corporation 2005-2006

#include <iostream>
#include <sstream>
#include <stdexcept>

#include <boost/filesystem/path.hpp>

#include <vpr/vpr.h>
#include <vpr/vprParam.h>

#if __VPR_version >= 1001005
#  include <vpr/IO/IOException.h>
#endif

#include <vpr/DynLoad/LibraryFinder.h>
#include <vpr/Util/Assert.h>

#include <IOV/PluginFactoryBase.h>


namespace fs = boost::filesystem;

namespace inf
{

void PluginFactoryBase::addScanPath(const std::vector<std::string>& scanPath)
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
                  std::cout << "IOV: Found plug-in [" << plugin_name << "]"
                            << std::endl;
               }
               else
               {
                  std::cout << "WARNING: IOV found a plug-in that was "
                            << "already registered: " << plugin_name
                            << std::endl;
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

vpr::LibraryPtr PluginFactoryBase::getPluginLibrary(const std::string& name)
   const
   throw (inf::NoSuchPluginException)
{
   plugin_libs_map_t::const_iterator lib = mPluginLibs.find(name);

   if ( lib != mPluginLibs.end() )
   {
      return (*lib).second;
   }
   else
   {
      std::ostringstream msg_stream;
      msg_stream << "No plug-in named '" << name << "' exists";
      throw NoSuchPluginException(msg_stream.str(), IOV_LOCATION);
   }
}

void PluginFactoryBase::registerCreatorFromName(const std::string& name)
   throw (inf::PluginLoadException)
{
   // Get the vpr::LibraryPtr for the named plug-in.  This will throw an
   // exception if name is not a valid plug-in name.
   vpr::LibraryPtr plugin_lib = getPluginLibrary(name);

   // At this point, we know that the given name must be a valid plug-in name.
   if ( plugin_lib->isLoaded() )
   {
      registerCreatorFromLib(plugin_lib, name);
   }
   else
   {
#if __VPR_version >= 1001010
      try
      {
         plugin_lib->load();
         registerCreatorFromLib(plugin_lib, name);
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
         registerCreatorFromLib(plugin_lib, name);
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

}
