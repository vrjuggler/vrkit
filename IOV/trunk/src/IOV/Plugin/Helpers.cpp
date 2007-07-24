// Copyright (C) Infiscape Corporation 2005-2007

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/exception.hpp>

#include <vpr/vpr.h>
#include <vpr/vprParam.h>
#include <vpr/System.h>
#include <vpr/DynLoad/LibraryFinder.h>
#include <vpr/Util/Assert.h>

#if __VPR_version >= 1001005
#  include <vpr/IO/IOException.h>
#endif

#include <IOV/Status.h>
#include <IOV/Plugin/Helpers.h>


namespace fs = boost::filesystem;

namespace inf
{

namespace plugin
{

std::vector<vpr::LibraryPtr>
findModules(const std::vector<std::string>& searchPath)
{
   std::vector<vpr::LibraryPtr> modules;

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
   // file name to be stripped off.  For a debug runtime build on Windows,
   // this would be "_d.dll".  For all other builds, it would be ".so",
   // ".dll", ".dylib", etc.  See above for the setting of driver_ext.
#if defined(IOV_DEBUG) && defined(_DEBUG) && defined(VPR_OS_Windows)
   const std::string strip_str = std::string("_d.") + driver_ext;
#else
   const std::string strip_str = std::string(".") + driver_ext;
#endif

   typedef std::vector<std::string>::const_iterator iter_type;
   for ( iter_type i = searchPath.begin(); i != searchPath.end(); ++i )
   {
      try
      {
         vpr::LibraryFinder finder(*i, driver_ext);
         typedef vpr::LibraryFinder::LibraryList lib_list_t;
         lib_list_t libs = finder.getLibraries();
         for (lib_list_t::const_iterator itr = libs.begin(); itr != libs.end(); ++itr)
         {
            try
            {
               (*itr)->load();
               modules.push_back(*itr);
            }
            catch(vpr::IOException& ex)
            {
               IOV_STATUS << ex.getDescription() << std::endl;
            }
         }
      }
      catch (std::exception& ex)
      {
         IOV_STATUS << "Exception scanning plug-in path: " << (*i) << std::endl
                    << ex.what() << std::endl;
      }
   }

   return modules;
}

std::string getPluginBaseDir()
{
   fs::path plugin_dir;

   std::string iov_base_dir;
   vpr::System::getenv("IOV_BASE_DIR", iov_base_dir);

   if ( ! iov_base_dir.empty() )
   {
      // TODO: Use a versioned IOV subdirectory.
      plugin_dir =
         fs::path(iov_base_dir, fs::native) / "lib" / "IOV" / "plugins";
   }

   return plugin_dir.native_directory_string();
}

std::vector<std::string>
buildSearchPath(const std::vector<std::string>& roots,
                const bool searchDebug)
{
   std::vector<std::string> search_path;

   typedef std::vector<std::string>::const_iterator iter_type;
   for ( iter_type r = roots.begin(); r != roots.end(); ++r )
   {
      try
      {
         fs::path cur_path(*r, fs::native);

         if ( fs::exists(cur_path) )
         {
#if defined(IOV_DEBUG) && ! defined(_DEBUG)
            if ( searchDebug )
            {
               fs::path dbg_path(cur_path / "debug");
               search_path.push_back(dbg_path.native_directory_string());
            }
#endif

            search_path.push_back(cur_path.native_directory_string());
         }
/*
         else
         {
            std::cerr << "NOTE: Plug-in path does not exist: "
                      << cur_path.native_directory_string() << std::endl;
         }
*/
      }
      catch (fs::filesystem_error& ex)
      {
         std::cerr << "ERROR: Failed to extend plug-in search path:\n"
                   << ex.what() << std::endl
                   << "WARNING: Plug-ins may not be found!" << std::endl;
      }
   }

   return search_path;
}

std::vector<std::string>
getDefaultSearchPath(const std::string& subdir, const bool searchDebug)
{
   std::vector<std::string> subdirs(1, subdir);
   return getDefaultSearchPath(subdirs, searchDebug);
}

void addDir(std::vector<std::string>& curDirs, const fs::path& basePath,
            const std::string& subdir)
{
   fs::path new_path(basePath / subdir);

   if ( fs::exists(new_path) && fs::is_directory(new_path) )
   {
      curDirs.push_back(new_path.native_directory_string());
   }
}

std::vector<std::string>
getDefaultSearchPath(const std::vector<std::string>& subdirs,
                     const bool searchDebug)
{
   std::vector<std::string> roots;

   // First, search everything under the 'plugins' subdirectory of the
   // current working directory.
   fs::path base_rel_path(std::string("plugins"));

   typedef std::vector<std::string>::const_iterator iter_type;
   for ( iter_type d = subdirs.begin(); d != subdirs.end(); ++d )
   {
      try
      {
         addDir(roots, base_rel_path, *d);
      }
      catch (fs::filesystem_error& ex)
      {
         std::cerr << "ERROR: Failed to add directory "
                   << base_rel_path.native_directory_string() << "/" << *d
                   << "to path:\n" << ex.what() << std::endl
                   << "WARNING: Some plug-ins may not be found!" << std::endl;
      }
   }

   // Next, search everything under the IOV plug-in base directory (if it is
   // known).
   const std::string base_dir = getPluginBaseDir();
   if ( ! base_dir.empty() )
   {
      fs::path base_env_path(base_dir, fs::native);

      for ( iter_type d = subdirs.begin(); d != subdirs.end(); ++d )
      {
         try
         {
            addDir(roots, base_env_path, *d);
         }
         catch (fs::filesystem_error& ex)
         {
            std::cerr << "ERROR: Failed to add directory "
                      << base_env_path.native_directory_string() << "/" << *d
                      << "to path:\n" << ex.what() << std::endl
                      << "WARNING: Some plug-ins may not be found!"
                      << std::endl;
         }
      }
   }

   return buildSearchPath(roots);
}

}

}
