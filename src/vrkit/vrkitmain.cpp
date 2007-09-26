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
            char* env_dir(NULL);
#if defined(_MSC_VER) && _MSC_VER >= 1400
            size_t len;
            _dupenv_s(&env_dir, &len, "VRKIT_BASE_DIR");
#else
            env_dir = std::getenv("VRKIT_BASE_DIR");
#endif

            try
            {
               fs::path base_dir;

               // If VRKIT_BASE_DIR is not set, look up the path to this DLL
               // and use it to provide a default setting for that environment
               // variable.
               if ( NULL == env_dir )
               {
                  char tmppath[1024];
                  std::memset(tmppath, 0, sizeof(tmppath));
                  GetModuleFileName(module, tmppath, sizeof(tmppath));

                  const fs::path dll_path(tmppath, fs::native);
                  base_dir = dll_path.branch_path().branch_path();
#if defined(VRKIT_DEBUG) && ! defined(_DEBUG)
                  // The debug DLL linked against the release runtime is in
                  // <base_dir>\lib\debug.
                  base_dir = base_dir.branch_path();
#endif

                  const std::string base_dir_str =
                     base_dir.native_directory_string();

#if defined(_MSC_VER) && _MSC_VER >= 1400
                  _putenv_s("VRKIT_BASE_DIR", base_dir_str.c_str());
#else
                  std::ostringstream env_stream;
                  env_stream << "VRKIT_BASE_DIR=" << base_dir_str;
                  putenv(env_stream.str().c_str());
#endif
               }
               else
               {
                  base_dir = fs::path(env_dir, fs::native);
#if defined(_MSC_VER) && _MSC_VER >= 1400
                  std::free(env_dir);
                  env_dir = NULL;
#endif
               }

#if defined(_MSC_VER) && _MSC_VER >= 1400
               _dupenv_s(&env_dir, &len, "VRKIT_DATA_DIR");
#else
               env_dir = std::getenv("VRKIT_DATA_DIR");
#endif

               // If VRKIT_DATA_DIR is not set, set a default relative to
               // base_dir.
               if ( NULL == env_dir )
               {
                  fs::path data_dir(base_dir / "share" / "vrkit");
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
                  env_dir = NULL;
               }
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1400
               _dupenv_s(&env_dir, &len, "VRKIT_PLUGINS_DIR");
#else
               env_dir = std::getenv("VRKIT_PLUGINS_DIR");
#endif

               // If VRKIT_PLUGINS_DIR is not set, set a default relative to
               // base_dir.
               if ( NULL == env_dir )
               {
                  fs::path plugin_dir(base_dir / "lib" / "vrkit" / "plugins");
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
                  env_dir = NULL;
               }
#endif
            }
            catch (fs::filesystem_error& ex)
            {
               std::cerr << "Automatic assignment of vrkit environment "
                         << "variables failed:\n" << ex.what() << std::endl;

#if defined(_MSC_VER) && _MSC_VER >= 1400
               if ( NULL != env_dir )
               {
                  std::free(env_dir);
               }
#endif
            }
         }
         break;
      default:
         break;
   }

   return TRUE;
}


#else /* Non-windows or..Unix case. */
#include <iostream>
#include <cstdlib>
#include <string>
#include <dlfcn.h>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/exception.hpp>

#include <vrkit/Version.h>


namespace fs = boost::filesystem;

extern "C" void __attribute ((constructor)) vrkitLibraryInit()
{
   fs::path base_dir;
   const char* env_dir = std::getenv("VRKIT_BASE_DIR");

   // If VRKIT_BASE_DIR is not set, look up the path to this shared library
   // and use it to provide a default setting for that environment variable.
   if ( NULL == env_dir )
   {
      Dl_info info;
      info.dli_fname = 0;
      const int result =
#if defined(__GNUC__) && __GNUC_MAJOR__ < 4
         dladdr((void*) &vrkitLibraryInit, &info);
#else
         dladdr(reinterpret_cast<void*>(&vrkitLibraryInit), &info);
#endif

      // NOTE: dladdr(3) really does return a non-zero value on success.
      if ( 0 != result )
      {
         try
         {
            fs::path lib_file(info.dli_fname, fs::native);
            lib_file = fs::system_complete(lib_file);

#if defined(VPR_OS_IRIX) && defined(_ABIN32)
            const std::string bit_suffix("32");
#elif defined(VPR_OS_IRIX) && defined(_ABI64) || \
      defined(VPR_OS_Linux) && defined(__x86_64__)
            const std::string bit_suffix("64");
#else
            const std::string bit_suffix("");
#endif

            // Get the directory containing this shared library.
            const fs::path lib_path = lib_file.branch_path();

            // Start the search for the root of the vrkit installation in the
            // parent of the directory containing this shared library.
            base_dir = lib_path.branch_path();

            // Use the lib subdirectory to figure out when we have found the
            // root of the vrkit installation tree.
            const fs::path lib_subdir(std::string("lib") + bit_suffix);

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

            if ( found )
            {
               setenv("VRKIT_BASE_DIR",
                      base_dir.native_directory_string().c_str(), 1);
            }
         }
         catch (fs::filesystem_error& ex)
         {
            std::cerr << "Automatic assignment of VRKIT_BASE_DIR failed:\n"
                      << ex.what() << std::endl;
         }
      }
   }
   else
   {
      try
      {
         base_dir = fs::path(env_dir, fs::native);
      }
      catch (fs::filesystem_error& ex)
      {
         std::cerr << "Invalid path set in VRKIT_BASE_DIR environment "
                   << "variable:\n" << ex.what() << std::endl;
      }
   }

   // If base_dir were empty, this would result in data_dir and plugin_dir
   // being relative to the current working directory.
   if ( ! base_dir.empty() )
   {
      std::string versioned_dir_name("vrkit");
#if defined(VRKIT_USE_VERSIONING)
      versioned_dir_name += std::string("-") +
                               std::string(vrkit::getVersion());
#endif

      // Construct the values for VRKIT_DATA_DIR and VRKIT_PLUGINS_DIR.
      const fs::path plugin_dir =
         base_dir / "lib" / versioned_dir_name / "plugins";
      const fs::path data_dir = base_dir / "share" / versioned_dir_name;

      // We use the overwrite value of 0 as a way around testing whether the
      // environment variable is already set.
      setenv("VRKIT_DATA_DIR", data_dir.native_directory_string().c_str(), 0);
      setenv("VRKIT_PLUGINS_DIR",
             plugin_dir.native_directory_string().c_str(), 0);
   }
}
#endif
