#!python

# vrkit is (C) Copyright 2005-2007
#    by Allen Bierbaum, Aron Bierbuam, Patrick Hartling, and Daniel Shipton
#
# This file is part of vrkit.
#
# vrkit is free software; you can redistribute it and/or modify it under the
# terms of the GNU Lesser General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#
# vrkit is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
# more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Import the stuff we need

import distutils.util
import sys, os, re, string, glob
pj = os.path.join

sys.path.insert(0,pj('deps','scons-addons','src'))
import SCons;
import SConsAddons.Options;   # Get the modular options stuff
import SConsAddons.Options.Boost
import SConsAddons.Options.VRJuggler.VRJ
import SConsAddons.Options.OpenSG
import SConsAddons.Util       # Get the utils

# Setup base scons options and settings
EnsureSConsVersion(0,96)
SetOption('max_drift',30)           # 30 second drift on MD5 Files
SetOption('implicit_cache',1)
SConsignFile()                      # Store all .sconsign stuff in single file

####  Local Helper methods ###
GetPlatform = SConsAddons.Util.GetPlatform;

def symlinkInstallFunc(dest, source, env):
    """Install a source file into a destination by sym linking it."""
    os.symlink(pj(os.getcwd(), source), dest)
    return 0

def BuildLinuxEnvironment():
   "Builds a base environment for other modules to build on set up for linux"
   env = Environment( ENV=os.environ)

   CXXFLAGS = ['-fmessage-length=0', '-Wall', '-fexceptions', '-Wunused',
               '-Wno-conversion']
   LINKFLAGS = []
   LIBPATH = []

   if optimize != 'no':
      CXXFLAGS.extend(['-DNDEBUG', '-O2'])
   else:
      CXXFLAGS.extend(['-D_DEBUG', '-gdwarf-2', '-fno-inline', '-fno-default-inline'])

   env.Append(
      CXXFLAGS    = CXXFLAGS,
      LINKFLAGS   = LINKFLAGS,
      LIBPATH     = LIBPATH
   )

   return env

def BuildDarwinEnvironment():
   "Builds a base environment for other modules to build on set up for Darwin"
   global optimize, profile, builders

   CXX = WhereIs('g++')
   CXXFLAGS = ['-fmessage-length=0', '-Wall', '-fexceptions', '-Wunused',
               '-Wno-conversion', '-Wno-long-double']
   LINKFLAGS = []

   # Enable profiling?
#   if profile != 'no':
#      CXXFLAGS.extend(['-pg'])
#      LINKFLAGS.extend(['-pg'])

   # Debug or optimize build?
   if optimize != 'no':
      CXXFLAGS.extend(['-DNDEBUG', '-O2'])
   else:
      CXXFLAGS.extend(['-D_DEBUG', '-g', '-fno-inline'])

   return Environment(
      ENV         = os.environ,
      CXX         = CXX,
      CXXFLAGS    = CXXFLAGS,
      LINKFLAGS   = LINKFLAGS
   )

optimize_default = 'no'
optimize = ARGUMENTS.get('optimize', optimize_default)
Export('optimize')

# --- Main Build --- #
help_text = "\n---- Build system ----\n";
if GetPlatform() == 'linux' or GetPlatform() == 'freebsd':
   baseEnv = BuildLinuxEnvironment()
elif GetPlatform() == 'mac':
   baseEnv = BuildDarwinEnvironment()

# Export symbols for use by SConscript files in subdirectories.
Export('GetPlatform')

# ---------- Options ------------ #
options_cache_filename = 'options.cache.' + distutils.util.get_platform()
opts = SConsAddons.Options.Options(files = [options_cache_filename, 'options.custom'],
                                   args= ARGUMENTS);
boost_options = SConsAddons.Options.Boost.Boost("boost", "1.32.0",
                                                required = False)
#boost_options.setUseVersion(True)
vrj_options = SConsAddons.Options.VRJuggler.VRJ.VRJ("vrjuggler", "1.1.169", 
                                                    required=True, useCppPath=True);
opensg_options = SConsAddons.Options.OpenSG.OpenSG("opensg", "1.6.0",
                                                   required = True, useCppPath=True)

Export('boost_options','opensg_options')

opts.AddOption(boost_options)
opts.AddOption( vrj_options )
opts.AddOption( opensg_options )

# Setup build dir and prefix settings
build_dir = "build." + distutils.util.get_platform()
def_prefix = pj( Dir('.').get_abspath(), build_dir, 'instlinks')
unspecified_prefix = "use-instlinks"
opts.Add('prefix', 'Installation prefix: def:' + unspecified_prefix, unspecified_prefix)
opts.Add('optimize', 'Enable optimization', optimize_default)

help_text += opts.GenerateHelpText(baseEnv)

# ----- Application Setup ------------ #
if not SConsAddons.Util.hasHelpFlag():
   opts.Update(baseEnv)
   opts.Save(options_cache_filename, baseEnv)

   # If defaulting to instlinks prefix:
   #  - Use symlinks
   #  - Manually set the used prefix to the instlinks of the build dir
   if baseEnv['prefix'] == unspecified_prefix:
      baseEnv['INSTALL'] = symlinkInstallFunc
      baseEnv['prefix'] = def_prefix

   # Setup a map with the paths that we will install everything to
   # - Note: this is used throughout the build to keep things consistent
   inst_paths = {}
   inst_paths['base'] = baseEnv['prefix']
   inst_paths['lib'] = pj(inst_paths['base'],'lib')
   inst_paths['lib_plugin'] = pj(inst_paths['lib'],'IOV','plugins')
   inst_paths['include'] = pj(inst_paths['base'],'include')
   inst_paths['share'] = pj(inst_paths['base'],'share','IOV')
   inst_paths['definitions'] = pj(inst_paths['share'],'definitions')
   inst_paths['app_base'] = pj(inst_paths['share'],'apps')
   inst_paths['test_base'] = pj(inst_paths['share'],'test')

   print "Using prefix: ", inst_paths["base"]

   # XXX: Hack around SConsAddons.Options.OpenSG.OpenSG not letting us provide
   # extra arguments to osg-config.  (Of course, if osg-config wasn't totally
   # braindead on Mac OS X, this wouldn't be such a problem.)
   if GetPlatform() == 'mac':
      mac_opts = os.popen(opensg_options.osgconfig_cmd + " --cflags System").read()
      opensg_options.found_cflags = mac_opts.strip().split(" ")

   # Setup the build environment
   # Add local directory to path and the "root" directory for this project
   vrj_options.updateEnv(baseEnv)
   opensg_options.updateEnv(baseEnv)

   # Incorporate the Boost options into baseEnv if Boost is available.  This
   # accounts for the case when a Boost installation is not available through
   # the output returned by 'vrjuggler-config --cxxflags'.
   if boost_options.isAvailable():
      boost_options.updateEnv(baseEnv)

   #if SCons.Util.WhereIs('distcc'):
   #   baseEnv.Prepend(CXX = "distcc ", CC = "distcc ")

   Export('baseEnv', 'build_dir', 'inst_paths')

   SConscript(dirs=['src'], build_dir=build_dir, duplicate=0)


   srcdir_abs_path = baseEnv.Dir('data').srcnode().abspath
   #print "src dir abs: ", srcdir_abs_path
   ret_args = []

   # Collect all file arguments from the walk into dirs and files
   def collect_args(junk, dir_path, name_list):
      dirs = []
      files = []
      for n in name_list:
         if os.path.isfile(os.path.join(dir_path, n)):
            files.append(n)
         else:
            dirs.append(n)
      ret_args.append( [dir_path, dirs, files] )

   os.path.walk(srcdir_abs_path, collect_args, None)

   # Install the contents of the data directory
   for p, d, files in ret_args:
      if not p.count('.svn'):
         for fname in [pj(p,f) for f in files]:
            strip_path = srcdir_abs_path
            if not strip_path.endswith('/'):
               strip_path += '/'
            base_fname = fname.replace(strip_path, '')
            baseEnv.InstallAs(pj(inst_paths['share'], 'data', base_fname),
                              fname)

   baseEnv.Alias('install', inst_paths['base'])

Help(help_text)

# end
