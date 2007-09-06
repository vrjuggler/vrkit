// vrkit is (C) Copyright 2005-2007
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

#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <string>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/exception.hpp>


namespace fs = boost::filesystem;

/**
 * Windows DLL entry point function. This ensures that the environment
 * variable \c VRKIT_BASE_DIR is set as soon as this DLL is attached to the
 * process. If it is not set, then it sets it based on an assumption about the
 * structure of a vrkit installation. More specifically, an assumption is made
 * that this DLL lives in the \c lib subdirectory of the vrkit installation.
 * Therefore, the root of the vrkit installation is the parent of the
 * directory containing this DLL.
 */
BOOL __stdcall DllMain(HINSTANCE module, DWORD reason, LPVOID reserved)
{
   switch (reason)
   {
      case DLL_PROCESS_ATTACH:
         {
            char tmppath[1024];
            std::memset(tmppath, 0, sizeof(tmppath));
            GetModuleFileName(module, tmppath, sizeof(tmppath));

            try
            {
               fs::path dll_path(tmppath, fs::native);
               fs::path base_dir = dll_path.branch_path().branch_path();
#if defined(VRKIT_DEBUG) && ! defined(_DEBUG)
               // The debug DLL linked against the release runtime is in
               // <base_dir>\lib\debug.
               base_dir = base_dir.branch_path();
#endif
               const std::string base_dir_str =
                  base_dir.native_directory_string();

               char* env_dir(NULL);
#if defined(_MSC_VER) && _MSC_VER >= 1400
               size_t len;
               _dupenv_s(&env_dir, &len, "VRKIT_BASE_DIR");
#else
               env_dir = std::getenv("VRKIT_BASE_DIR");
#endif

               if ( NULL == env_dir )
               {
#if defined(_MSC_VER) && _MSC_VER >= 1400
                  _putenv_s("VRKIT_BASE_DIR", base_dir_str.c_str());
#else
                  std::ostringstream env_stream;
                  env_stream << "VRKIT_BASE_DIR=" << base_dir_str;
                  putenv(env_stream.str().c_str());
#endif
               }
#if defined(_MSC_VER) && _MSC_VER >= 1400
               else
               {
                  std::free(env_dir);
               }
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1400
               _dupenv_s(&env_dir, &len, "VRKIT_DATA_DIR");
#else
               env_dir = std::getenv("VRKIT_DATA_DIR");
#endif

               if ( NULL == env_dir )
               {
                  fs::path data_dir = base_dir / "share" / "vrkit";
                  const std::string data_dir_str =
                     data_dir.native_directory_string();

#if defined(_MSC_VER) && _MSC_VER >= 1400
                  _putenv_s("VRKIT_DATA_DIR", data_dir_str.c_str());
#else
                  std::ostringstream env_stream;
                  env_stream << "VRKIT_DATA_DIR=" << data_dir_str;
                  putenv(env_stream.str().c_str());
#endif
               }
#if defined(_MSC_VER) && _MSC_VER >= 1400
               else
               {
                  std::free(env_dir);
               }
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1400
               _dupenv_s(&env_dir, &len, "VRKIT_PLUGINS_DIR");
#else
               env_dir = std::getenv("VRKIT_PLUGINS_DIR");
#endif

               if ( NULL == env_dir )
               {
                  fs::path plugin_dir = base_dir / "lib" / "vrkit" / "plugins";
                  const std::string plugin_dir_str =
                     plugin_dir.native_directory_string();

#if defined(_MSC_VER) && _MSC_VER >= 1400
                  _putenv_s("VRKIT_PLUGINS_DIR", plugin_dir_str.c_str());
#else
                  std::ostringstream env_stream;
                  env_stream << "VRKIT_PLUGINS_DIR=" << plugin_dir_str;
                  putenv(env_stream.str().c_str());
#endif
               }
#if defined(_MSC_VER) && _MSC_VER >= 1400
               else
               {
                  std::free(env_dir);
               }
#endif
            }
            catch (fs::filesystem_error& ex)
            {
               std::cerr << "Automatic assignment of vrkit environment "
                         << "variables failed:\n" << ex.what() << std::endl;
            }
         }
         break;
      default:
         break;
   }

   return TRUE;
}


#else /* Non-windows or..Unix case. */
#include <dlfcn.h>
#include <string>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/exception.hpp>

#include <vpr/System.h>

#include <vrkit/Status.h>
#include <vrkit/Version.h>

namespace fs = boost::filesystem;

extern "C" void __attribute ((constructor)) vrkit_library_init()
{
   Dl_info info;
   info.dli_fname = 0;
   const int result =
      dladdr(reinterpret_cast<const void*>(&vrkit_library_init), &info);

   // NOTE: dladdr(3) really does return a non-zero value on success.
   if ( 0 != result )
   {
      try
      {
         fs::path lib_file(info.dli_fname, fs::native);
         lib_file = fs::system_complete(lib_file);

         // Get the directory containing this shared library.
         const fs::path lib_path = lib_file.branch_path();

         // Start the search for the root of the vrkit installation in the
         // parent of the directory containing this shared library.
         fs::path base_dir = lib_path.branch_path();

         // Use the lib subdirectory to figure out when we have found the root
         // of the vrkit installation tree.
         const fs::path lib_subdir(std::string("lib"));

         bool found(false);
         while ( ! found && ! base_dir.empty() )
         {
            try
            {
               if ( ! fs::exists(base_dir / lib_subdir) )
               {
                  base_dir = base_dir.branch_path();
               }
               else
               {
                  found = true;
               }
            }
            catch (fs::filesystem_error&)
            {
               base_dir = base_dir.branch_path();
            }
         }
         if( found )
         {
            // Construct VRKIT_DATA_DIR and VRKIT_PLUGINS_DIR
            std::string vrkit_versioned_dir_name = "vrkit";
#ifdef VRKIT_USE_VERSIONING
            vrkit_versioned_dir_name.append("-");
            vrkit_versioned_dir_name.append(vrkit::getVersion());
#endif

            // Go from /base to base/lib/versioned_vrkit_dir/plugins
            fs::path plugins_dir = base_dir / "lib" /
                                         vrkit_versioned_dir_name / "plugins";
            // Go from /base to /base/share/versioned_vrkit_dir
            fs::path data_dir = base_dir / "share" /
                                         vrkit_versioned_dir_name;

            std::string vrkit_base_dir_env_var;
            vpr::System::getenv("VRKIT_BASE_DIR", vrkit_base_dir_env_var);
            if ( vrkit_base_dir_env_var.empty() )
            {
               vpr::System::setenv("VRKIT_BASE_DIR", base_dir.string());
               VRKIT_STATUS << "VRKIT_BASE_DIR set to: "
                            << base_dir.string() << std::endl;
            }

            std::string vrkit_data_dir_env_var;
            vpr::System::getenv("VRKIT_DATA_DIR", vrkit_data_dir_env_var);
            if ( vrkit_data_dir_env_var.empty() )
            {
               vpr::System::setenv("VRKIT_DATA_DIR", data_dir.string());
               VRKIT_STATUS << "VRKIT_DATA_DIR set to: "
                            << data_dir.string() << std::endl;
            }

            std::string vrkit_plugin_dir_env_var;
            vpr::System::getenv("VRKIT_PLUGINS_DIR", vrkit_plugin_dir_env_var);
            if ( vrkit_plugin_dir_env_var.empty() )
            {
               vpr::System::setenv("VRKIT_PLUGINS_DIR", plugins_dir.string());
               VRKIT_STATUS << "VRKIT_PLUGINS_DIR set to: "
                            << plugins_dir.string() << std::endl;
            }
         }
      }
      catch (fs::filesystem_error& ex)
      {
         std::cerr << "Automatic assignment of VRKIT_BASE_DIR failed:\n"
                   << ex.what() << std::endl;
      }
   }
}
#endif
