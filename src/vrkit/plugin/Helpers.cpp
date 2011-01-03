// vrkit is (C) Copyright 2005-2011
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

#include <boost/algorithm/string/predicate.hpp>
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

#include <vrkit/Status.h>
#include <vrkit/Version.h>
#include <vrkit/plugin/Helpers.h>


namespace fs = boost::filesystem;

namespace vrkit
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

   typedef std::vector<std::string>::const_iterator iter_type;
   for ( iter_type i = searchPath.begin(); i != searchPath.end(); ++i )
   {
      try
      {
         vpr::LibraryFinder finder(*i, driver_ext);
         typedef vpr::LibraryFinder::LibraryList lib_list_t;
         typedef lib_list_t::const_iterator iter_type;
         lib_list_t libs = finder.getLibraries();
         for ( iter_type itr = libs.begin(); itr != libs.end(); ++itr )
         {
#if defined(VPR_OS_Windows)
            const bool is_debugrt(boost::iends_with((*itr)->getName(),
                                                    "_d.dll"));

            // With a debug runtime build on Windows, we only want plug-ins
            // named as "_d.dll".
#if defined(VRKIT_DEBUG) && defined(_DEBUG)
            if ( ! is_debugrt )
#else
            // In all other cases on Windows, we want to skip plug-ins named
            // as "_d.dll".
            if ( is_debugrt )
#endif
            {
               continue;
            }
#endif

#if __VPR_version >= 1001005
            try
            {
               (*itr)->load();
               modules.push_back(*itr);
            }
            catch (vpr::IOException& ex)
            {
               VRKIT_STATUS << ex.getDescription() << std::endl;
            }
#else
            if ( (*itr)->load().success() )
            {
               modules.push_back(*itr);
            }
#endif
         }
      }
      catch (std::exception& ex)
      {
         VRKIT_STATUS << "Exception scanning plug-in path: " << (*i)
                      << std::endl
                    << ex.what() << std::endl;
      }
   }

   return modules;
}

std::string getPluginBaseDir()
{
   fs::path plugin_dir;

   std::string vrkit_plugins_dir;
   vpr::System::getenv("VRKIT_PLUGINS_DIR", vrkit_plugins_dir);

   if ( ! vrkit_plugins_dir.empty() )
   {
      plugin_dir = fs::path(vrkit_plugins_dir, fs::native);
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
// If this is a debug-enabled build, search the "debug" subdirectory of
// cur_path before cur_path.
#if defined(VRKIT_DEBUG)
            // Search the "debug" subdirectory if we were told to do so.
            if ( searchDebug )
            {
               fs::path dbg_path(cur_path / "debug");
               search_path.push_back(dbg_path.native_directory_string());
            }

            search_path.push_back(cur_path.native_directory_string());

// If this is not a debug-enabled build, search the "debug" subdirectory of
// cur_path after cur_path.
#else
            search_path.push_back(cur_path.native_directory_string());

            // Search the "debug" subdirectory if we were told to do so.
            if ( searchDebug )
            {
               fs::path dbg_path(cur_path / "debug");
               search_path.push_back(dbg_path.native_directory_string());
            }
#endif
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

   // Next, search everything under the vrkit plug-in base directory (if it is
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
