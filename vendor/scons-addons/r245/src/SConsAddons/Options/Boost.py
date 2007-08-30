"""SConsAddons.Options.Boost

Definds options for boost project
"""

__revision__ = "__FILE__ __REVISION__ __DATE__ __DEVELOPER__"

#!python
# SCons based build file for Boost
# Base file
import SCons.Environment
import SCons
import SConsAddons.Options
import SConsAddons.Util as sca_util
import SCons.Util
import distutils.sysconfig

import string
import sys
import os
import re

pj = os.path.join

from SCons.Util import WhereIs

import SCons.SConf
Configure = SCons.SConf.SConf
# ##############################################
# Options
# ##############################################
class Boost(SConsAddons.Options.PackageOption):
   def __init__(self, name, requiredVersion, 
                useDebug=False, useMt=True, libs=[], 
                required=True, useCppPath=False, 
                toolset="auto", useVersion=False,
                preferDynamic=True, autoLink = False):
      """
         name - The name to use for this option
         requiredVersion - The version of Boost required (ex: "1.30.0")
         useDebug - Should we use debug boost libraries [default: False]
         useMt - Should we use multi-threaded boost libraries [default: True]
         libs - Boost libraries needed that are actually compiled (base library names. ex: python)
         required - Is the dependency required?  (if so we exit on errors)
         useCppPath - If true, then include path is added to CPPPATH if not, then added to CPPFLAGS directly
	 toolset - The toolset name to use (ex: "auto","gcc", "il")
         preferDynamic - If true, prefer linking against a dynamic library.
         autoLink - If true and using msvc, then attempt to use boost's autolinking capabilies.
      """
      help_text = ["Base directory for Boost. include, and lib should be under this directory.",
                   "Include directory for boost (if not under base)."]
      self.baseDirKey = "BoostBaseDir"
      self.incDirKey = "BoostIncludeDir"
      self.requiredVersion = requiredVersion
      self.libVersionStr = None
      self.lib_names = libs
      self.required = required
      self.useCppPath = useCppPath
      self.toolset = toolset
      self.preferDynamic = preferDynamic
      self.autoLink = autoLink
      if sca_util.GetPlatform() != "win32":
         self.autoLink = False
      SConsAddons.Options.PackageOption.__init__(self, name, 
                                                 [self.baseDirKey, self.incDirKey], 
                                                 help_text)
      self.available = False            # Track availability
      
      self.found_incs = []
      self.found_incs_as_flags = ""     # The includes as flags to add on command line
      self.found_lib_paths = []
      self.found_defines = []

      # configurable options
      self.baseDir = None
      self.incDir = None
      self.setupLibrarySettings()
      
      # Options for which libraries to use
      self.use_mt = useMt
      self.use_debug = useDebug 
      self.use_ver = useVersion
      #print "Use debug set to:", self.use_debug

   def setToolset(self, toolset):
      self.toolset = toolset
   
   def setUseMt(self, bval=True):
      self.use_mt = bval
      
   def setUseDebug(self, bval=True):
      self.use_debug = bval

   def setUseVersion(self, useVersion = True):
      self.use_ver = useVersion   

   def setupLibrarySettings(self):
      " Setup some default settings and data that will be used on this platform. "
      # Map from library name to header to check for       
      self.headerMap = { 'program_options':'boost/program_options.hpp',
                         #'python':'boost/python.hpp',
                         'python':'boost/python/enum.hpp',
                         'signals':'boost/signal.hpp',
                         'thread':'boost/thread.hpp',
                         'filesystem':'boost/filesystem/path.hpp' }
      
      # Map for extra libs needed for config test
      self.extraEnvOptions = {}

      # --- Build up settings using distutils.sysconfig to get Python build options --- #
      # distutils.sysconfig.get_config_vars()
      try:
         self.python_version = distutils.sysconfig.get_python_version()    # ex: '2.3'
      except:
         self.python_version = distutils.sysconfig.get_config_var("VERSION")    # ex: '2.3'
      self.python_inc_dir = distutils.sysconfig.get_python_inc()
      if sca_util.GetPlatform() == "win32":
         self.python_embedded_link_flags = []
         self.python_lib_path = [pj(sys.prefix,'libs'),]
         self.python_static_lib_path = [""]                   # There is no static lib on win32
         lib_python_fname = 'python' + self.python_version.replace('.','')
         self.python_extra_libs = [lib_python_fname,]
         #self.python_link_flags = ["/NODEFAULTLIB:"+lib_python_fname+"_d",]
         self.python_link_flags = []
         self.thread_extra_libs = []
      else:
         # Link flags that may be needed on unix for the embedded case
         #self.python_embedded_link_flags = [distutils.sysconfig.get_config_var('LINKFORSHARED'),]
         self.python_embedded_link_flags = ["-Wl,-export-dynamic"]
         self.python_lib_path = [pj(sys.prefix,'lib'),]
         self.python_static_lib_path = distutils.sysconfig.get_python_lib(standard_lib=True) + "/config"         
         lib_python_fname = 'python' + self.python_version
         self.python_extra_libs = [lib_python_fname, "util", "pthread", "dl"]  # See SHLIBS
         self.python_link_flags = []
         self.thread_extra_libs = ["pthread","dl"]
      
   def buildFullLibName(self, libname, env, useDebug=None):
      """ Returns the full name of the boost library.
          TODO: Do a better job of finding the libraries naming convention
          in the validate method above.
      """
      fullname = "boost_" + libname
      debug_ext = "-d"
      if sca_util.GetPlatform() == "win32":
         debug_ext = "-gd"     # Hack for now assuming debug code and runtime
      if sca_util.GetPlatform() != 'darwin':
         fullname = fullname + "-" + self.toolset
         if self.use_mt:
            fullname += "-mt"

         if useDebug is None:
            if self.use_debug:
               fullname += debug_ext
            elif env and env.has_key("variant") and env["variant"].has_key("type"):
               var_type = env["variant"]["type"]
               if "debug" == var_type:
                  fullname += debug_ext
         elif useDebug is True:
            fullname += debug_ext


      if self.use_ver and self.libVersionStr is not None:
         fullname += "-" + self.libVersionStr

      return fullname
         
   def startProcess(self):
      """ Called at beginning of processing.  Perform any intialization or notification here. """
      print "Updating ", self.name
       
   def setInitial(self, optDict):
      " Set initial values from given options dictionary. "
      print "Loading initial settings for boost"
      if optDict.has_key(self.baseDirKey):
         self.baseDir = optDict[self.baseDirKey]
         print "   %s specified or cached. [%s]."% (self.baseDirKey, self.baseDir)
      if optDict.has_key(self.incDirKey):
         self.incDir = optDict[self.incDirKey]
         print "   %s specified or cached. [%s]."% (self.incDirKey, self.incDir)

   def find(self, env):
      """ If base dir was not specified, attempt to find boost
          using some very basic searches.
      """
      # Quick exit if nothing to find
      if self.baseDir != None:
         return
      
      ver_header = None

      # Find boost/version.hpp
      print "   searching for boost..."
      if env.Dictionary().has_key('CPPPATH'):
         print "      Searching CPPPATH..."
         ver_header = env.FindFile(pj('boost','version.hpp'), env['CPPPATH'])

      if (None == ver_header) and env.Dictionary().has_key('CPLUS_INCLUDE_PATH'):
         print "      Searching CPLUS_INCLUDE_PATH..."
         ver_header = SCons.Script.SConscript.FindFile(pj('boost', 'version.hpp'),
                                    string.split(env['ENV']['CPLUS_INCLUDE_PATH'], os.pathsep))
         
      if None == ver_header:
         self.checkRequired("   could not find boost/version.hpp.")
      else:
         ver_header = str(ver_header)
         print "   found boost/version.hpp.\n"

         # find base dir
         self.baseDir = os.path.dirname(os.path.dirname(os.path.dirname(ver_header)))
         if not os.path.isdir(self.baseDir):
            self.checkRequired("   returned directory does not exist:%s"% self.baseDir)
            self.baseDir = None
         else:
            print "   found at: ", self.baseDir

   def validate(self, env):
      " Check to make sure that the current settings work and are valid. """
      # Check that path exist
      # Check that an include file: boost/version.hpp  exists
      passed = True
      self.available = False
      
      if not self.baseDir:
         self.checkRequired("Boost base dir not set")
         return
      
      if not os.path.isdir(self.baseDir):    # If we don't have a directory
         self.checkRequired("Boost base dir is not a directory: %s" % self.baseDir)
         return
      
      # --- Determine toolset --- #
      if self.toolset == "auto":
         print "   Boost, autofinding toolset... ",
         
         if env["CC"] == "gcc":
            if sca_util.GetPlatform() == 'darwin':
               self.toolset = "darwin"
            else:
               self.toolset = "gcc"
         elif env["CC"] == "cl" and env.has_key("MSVS"):
            ver = env["MSVS"]["VERSION"]
            if "7.1" == ver:
               self.toolset = "vc71"
            elif "7.0" == ver:
               self.toolset = "vc7"
         elif sca_util.GetPlatform() == 'darwin' and env['CC'] == 'cc':
            self.toolset = "darwin"
         else:
            self.checkRequired("Could not auto determine boost toolset.")
            return
         
         print " toolset: [%s]"%self.toolset

      # --- Find include path --- #
      # If inc dir not set, try to find it      
      # - Try just include, try local, then try subdirs in reverse sorted order (to get most recent)
      if not self.incDir:
         print "   Searching for correct boost include dir...",
         base_include_dir = pj(self.baseDir, 'include')
         potential_dirs = [base_include_dir, self.baseDir]
         if os.path.isdir(base_include_dir):
            inc_dirs = [pj(base_include_dir,d) for d in os.listdir(base_include_dir)]
            inc_dirs.sort()
            inc_dirs.reverse()
            potential_dirs.extend(inc_dirs)
         
         for d in potential_dirs:
            if os.path.isfile(pj(d,'boost','version.hpp')):
               self.incDir = d
               break
         
         if self.incDir:
            print "  found: ", self.incDir
         else:
            print "  not found."
            self.checkRequired("Can not find boost include directory.")
            return

      # user specified, verify it is a directory
      elif not os.path.isdir(pj(self.incDir,'boost')):
         self.checkRequired("Boost inc dir is not a valid directory: %s" % pj(self.incDir,'boost'))
         return
      
      # Check the version header is there         
      version_header = pj(self.incDir, 'boost', 'version.hpp')         
      if not os.path.isfile(version_header):
         self.checkRequired("Boost version.hpp header does not exist:%s"%version_header)
         return
         
      print "   boost header path: ", self.incDir
      
      # --- Check version requirement --- #
      ver_file = file(version_header)
      ver_file_contents = ver_file.read()
      ver_match = re.search("define\s+?BOOST_VERSION\s+?(\d*)",
                            ver_file_contents)
      if not ver_match:
         self.checkRequired("   could not find BOOST_VERSION in file: %s"%version_header)
         return

      lib_ver_match = re.search("define\s+?BOOST_LIB_VERSION\s+?\"(.*)\"",
                                ver_file_contents)
      if lib_ver_match:
         self.libVersionStr = lib_ver_match.group(1)
      else:         
         print "WARNING: Could not determine library version string"
         self.libVersionStr = None

      found_ver_str = int(ver_match.group(1))
      found_ver_str = str(found_ver_str / 100000) + '.' + str(found_ver_str / 100 % 1000) + '.' + str(found_ver_str % 100)
      req_ver = [int(n) for n in self.requiredVersion.split('.')]
      found_ver = [int(n) for n in found_ver_str.split('.')]
      print "   boost version:", ".".join([str(x) for x in found_ver])
      if found_ver < req_ver:
         self.checkRequired("   found version is to old: required:%s found:%s"%(self.requiredVersion,found_ver_str))
         return

      # Set lists of the options we want
      self.found_incs = [self.incDir]
      self.found_incs_as_flags = [env["INCPREFIX"] + p for p in self.found_incs];
      self.found_lib_paths = [pj(self.baseDir, 'lib')]
      self.found_defines = []
      # Note: This doesn't work because the configure context uses the static
      #       run-time and this makes boost error out.
      #if self.preferDynamic:
      #   self.found_defines.append("BOOST_ALL_DYN_LINK")      
      #if not self.autoLink:
      #   self.found_defines.append("BOOST_ALL_NO_LIB")

      ######## BUILD CHECKS ###########  
      # --- Check building against libraries --- #   

      # For each library, find cononical lib name and associated header to check
      # default to checking lib with config.hpp
      for libname in self.lib_names:
         full_libname = self.buildFullLibName(libname,env)
         header_to_check = 'boost/config.hpp'
         if self.headerMap.has_key(libname):
            header_to_check = self.headerMap[libname]

         # Create config environment
         # - Need to extend the environment
         conf_env = env.Copy()
         conf_env.Append(CPPPATH= self.found_incs, 
                         LIBPATH = self.found_lib_paths,
                         CPPDEFINES = self.found_defines)
         if "python" == libname:
            conf_env.Append(CPPPATH = self.python_inc_dir,
                            LIBPATH = self.python_lib_path,
                            #LIBS = [full_libname,] + self.python_extra_libs
                            LIBS = self.python_extra_libs
                         )
         
         # Thread library needs some additional libraries on Linux... (yuck)
         if "thread" == libname:
            conf_env.Append(LIBS = [full_libname,] + self.thread_extra_libs)
         
         conf_ctxt = Configure(conf_env)
         result = conf_ctxt.CheckLibWithHeader(full_libname, header_to_check, "c++")
           
         if not result:
            passed = False
            self.checkRequired("Can't compile test program: lib: %s full_lib: %s header:%s"%(libname,full_libname,header_to_check))
            
         conf_ctxt.Finish()

      # --- Handle final settings ---- #     
      if not passed:
         # Clear everything
         self.baseDir = None
         self.incDir = None
         edict = env.Dictionary()
         for k in (self.baseDirKey, self.incDirKey):
            if edict.has_key(k):
               del edict[k]
         self.found_incs = None
         self.found_lib_paths = []
         self.found_defines = []
      else:
         self.available = True


   def apply(self, env, libs=None, useCppPath=False, useDebug=None):
      """ Add environment options for building against Boost libraries.
          Apply the options and take into account any variant information
          in the environment.
      """
      if self.found_incs:
         if self.useCppPath or useCppPath:
            env.AppendUnique(CPPPATH = self.found_incs)
         else:
            env.AppendUnique(CXXFLAGS = self.found_incs_as_flags)
      env.AppendUnique(CPPDEFINES = self.found_defines,
                       LIBPATH = self.found_lib_paths)
      if not self.autoLink:
         full_libs = [self.buildFullLibName(l,env, useDebug) for l in self.lib_names if 'python' != l]         
         env.AppendUnique(LIBS = full_libs)      


   def updatePythonEmbeddedEnv(self,env):
      self.applyPythonEmbeddedEnv(env)

   def applyPythonEmbeddedEnv(self,env, useDebug=None):
      """ Update the environment for building python embedded. 
          XXX: may need python_static_lib_path.
      """
      self.apply(env)
      #print "Full python lib name:", self.buildFullLibName('python')
      env.AppendUnique(LIBS = [self.buildFullLibName('python',env, useDebug)])
      env.AppendUnique(CPPPATH = [self.python_inc_dir,],
                       LINKFLAGS = self.python_embedded_link_flags + self.python_link_flags,
                       LIBPATH = self.python_lib_path,
                       LIBS = self.python_extra_libs)

   
   def updatePythonModEnv(self, env, useDebug=None):
      self.applyPythonModEnv(env, useDebug)
   
   def applyPythonModEnv(self, env, useDebug=None):
      """ Update the environment for building python modules """
      if not "python" in self.lib_names:
         print "Tried to updatePythonModEnv with boost option object not configured with python library.\n"
         sys.exit(0)
         
      self.apply(env)
      env.AppendUnique(LIBS = [self.buildFullLibName("python",env, useDebug),] )    # Add on the boost python library
      env.AppendUnique(CPPPATH = [self.python_inc_dir,],
                       LINKFLAGS = self.python_link_flags,
                       LIBPATH = self.python_lib_path,
                       LIBS = self.python_extra_libs)

            
      env["SHLIBPREFIX"] = ""                    # Clear the library prefix settings
      if sca_util.GetPlatform() == "win32":
         env["SHLIBSUFFIX"] = ".pyd"
         
      if(sca_util.GetPlatform() == "linux"):
         env['CXXCOM'] += " ; objcopy --set-section-flags .debug_str=contents,debug $TARGET"
         env['SHCXXCOM'] += " ; objcopy -v --set-section-flags .debug_str=contents,debug $TARGET $TARGET"
      
      # Add visibility flags for gcc 4.0 and greater
      if "g++" in env["CXX"]:
         gcc_version = env["CXXVERSION"].split(".")
         if int(gcc_version[0]) >= 4:         
            env.AppendUnique(CXXFLAGS = ['-fvisibility=hidden', '-fvisibility-inlines-hidden'])

   def getSettings(self):
      return [(self.baseDirKey, self.baseDir), (self.incDirKey, self.incDir)]
   
   def dumpSettings(self,env=None):
      "Write out the settings"
      print "BoostBaseDir:", self.baseDir
      print "BoostIncludeDir:", self.incDir
      print "CPPPATH (as flags):", self.found_incs_as_flags
      print "CPPDEFINES:", self.found_defines
      print "LIBS:", self.lib_names
      print "LIBS: (full):", [self.buildFullLibName(l,env) for l in self.lib_names]
      print "LIBPATH:", self.found_lib_paths      
      print "Python settings"
      print "               inc:", self.python_inc_dir
      print "     embedded link:", self.python_embedded_link_flags
      print "               lib:", self.python_extra_libs
      print "   static lib path:", self.python_static_lib_path
