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
import sys, os, re, string, glob, types
pj = os.path.join

sys.path.insert(0,pj('deps','scons-addons','src'))
import SCons;
import SConsAddons.Options as sca_opts   # Get the modular options stuff
import SConsAddons.Options.Boost
import SConsAddons.Options.VRJuggler.VRJ
import SConsAddons.Options.OpenSG
import SConsAddons.Variants as sca_variants
import SConsAddons.Builders

from SConsAddons.EnvironmentBuilder import EnvironmentBuilder
import SConsAddons.Util as sca_util      # Get the utils
GetPlatform = sca_util.GetPlatform

# Setup base scons options and settings
EnsureSConsVersion(0,96)
SetOption('max_drift',30)           # 30 second drift on MD5 Files
SetOption('implicit_cache',1)
SConsignFile()                      # Store all .sconsign stuff in single file

# --- Main Build --- #
VRKIT_VERSION = sca_util.GetVersionFromHeader('VRKIT', 'src/vrkit/Version.h')
vrkit_version_str = '%i.%i.%i' % VRKIT_VERSION
Export('VRKIT_VERSION')
print 'Building vrkit version: %s' % vrkit_version_str

if GetPlatform() == 'win32':
   # XXX: Temp hack to get msvs version setting
   if ARGUMENTS.has_key('MSVS_VERSION'):
      opt_env = Environment(MSVS_VERSION=ARGUMENTS['MSVS_VERSION'])
   else:
      opt_env = Environment()

   # When using Visual C++ 8.0 or newer, embed the manifest in all DLLs and
   # EXEs.
   # NOTE: The [:3] bit used before passing the MSVS_VERSION value to float()
   # is to handle the case of Visual C++ Express Edition which appends "Exp"
   # to the version number.
   if opt_env.has_key('MSVS_VERSION') and float(opt_env['MSVS_VERSION'][:3]) >= 8.0:
      opt_env['SHLINKCOM'] = \
         [opt_env['SHLINKCOM'],
          'mt.exe -manifest ${TARGET}.manifest -outputresource:$TARGET;2']
      opt_env['LINKCOM'] = \
         [opt_env['LINKCOM'],
          'mt.exe -manifest ${TARGET}.manifest -outputresource:$TARGET;1']
else:
   opt_env = Environment(ENV = os.environ)

# Export symbols for use by SConscript files in subdirectories.
platform = GetPlatform()
Export('GetPlatform', 'platform')
unspecified_prefix = 'use-instlinks'

SConsAddons.Builders.registerSubstBuilder(opt_env)

# ---------- Options ------------ #
options_cache_filename = 'options.cache.' + distutils.util.get_platform()
opts = SConsAddons.Options.Options(files = [options_cache_filename, 'options.custom'],
                                   args= ARGUMENTS)

build_options = {}
build_options["versioning"] = sca_opts.BoolOption(
   "versioning", "Install vrkit as a versioned install", True)
build_options["install_prefix"] = sca_opts.SimpleOption(
    "prefix", "prefix", "Installation prefix", unspecified_prefix, None, None, None)
#build_options["build_suffix"] = sca_opts.SimpleOption(
#    "build_suffix", "build_suffix",
#    "Suffix to append to build directory.  Useful for compiling multiple variations on the same platform",
#    "", None, None, None)
build_options["enable_distcc"] = sca_opts.BoolOption(
    "enable_distcc", "Enable use of distcc during build. (distcc must be in your path)", False)

boost_options = SConsAddons.Options.Boost.Boost("boost", "1.33.0",
                                                libs = ['signals', 'program_options'],
                                                required = True,
                                                useVersion = True,
                                                autoLink = True)

vrj_options = SConsAddons.Options.VRJuggler.VRJ.VRJ("VR Juggler", "2.0.1")
opensg_options = SConsAddons.Options.OpenSG.OpenSG("opensg", "1.8.0",
                                                   required = True,
                                                   useCppPath = True)

ffmpeg_options = \
   SConsAddons.Options.FlagPollBasedOption.FlagPollBasedOption("AvFormat",
                                                               "libavformat",
                                                               "51.0.0",
                                                               False, True)

if "win32" == platform:
   opts.AddOption(
      sca_opts.BoolOption("enable_vfw", "Enable the use of Video for Windows",
                          True)
   )
   opts.AddOption(
      sca_opts.BoolOption("enable_dshow", "Enable the use of DirectShow",
                          True)
   )

   # Options related to using FFmpeg compiled for use with MinGW.
   opts.Add(
      "mingw",
      "MinGW installation directory (only needed if using FFmpeg from MinGW)."
   )
   opts.Add(
      "mingw_gcc_lib",
      "Directory containing MinGW libgcc.a (only needed if using FFmpeg from MinGW)."
   )

   mingw_ffmpeg = \
      sca_opts.StandardPackageOption("mingw_ffmpeg", 
                                     "Use FFmpeg build for MinGW use.")
   opts.AddOption(mingw_ffmpeg)
   Export('mingw_ffmpeg')

   opts.Add("platform_sdk", "Microsoft Platform SDK location",
            default = r"C:\Program Files\Microsoft Platform SDK")

Export('boost_options','opensg_options', 'ffmpeg_options', 'vrj_options')

opts.AddOption(boost_options)
opts.AddOption(vrj_options)
opts.AddOption(opensg_options)
opts.AddOption(ffmpeg_options)

# Create variant helper and builder. We only vary on type (optimized/debug)
# and architecture. This is because no static libraries are compiled.
variant_helper = sca_variants.VariantsHelper(variantKeys = ["type", "arch"])
base_bldr = EnvironmentBuilder()
variant_helper.addOptions(opts)
base_bldr.addOptions(opts)

#
# 2) Register options
#    This should not require any changes unless a new option group is added.
opts.AddOption(sca_opts.SeparatorOption("\nBuild/Install settings"))
for opt in build_options.itervalues():
   opts.AddOption(opt)

#opts.AddOption(sca_opts.SeparatorOption("\nPackage settings (required libs)"))
#for opt in required_libs_options.itervalues():
#   opts.AddOption(opt)

#Export('required_libs_options', 'build_options')
Export('build_options')

help_text = """---- vrkit Build System ---- 

%s

This file will be loaded each time.  Note: Options are cached in the file: %s
""" %(opts.GenerateHelpText(opt_env),options_cache_filename)


#viewerEnv.Append(LIBS = ['avcodec', 'avformat', 'avutil', 'postproc'])

# ----- Application Setup ------------ #
if not sca_util.hasHelpFlag():
   opts.Process(opt_env)

   # Try to save the options if possible
   try:
      opts.Save(options_cache_filename, opt_env)
   except LookupError, le:
      pass

   # -- Common builder settings
   variant_helper.readOptions(opt_env)
   base_bldr.readOptions(opt_env)
   base_bldr.enableWarnings()      

   if opt_env["enable_distcc"] and sca_util.WhereIs("distcc"):
      opt_env.Prepend(CXX = "distcc ", CC = "distcc ")

   # Create base build directory.
   base_build_dir = "build." + platform
   if opt_env.has_key("MSVS"):
      base_build_dir += "." + opt_env["MSVS"]["VERSION"]

   # If defaulting to instlinks prefix:
   #  - Use symlinks
   #  - Manually set the used prefix to the instlinks of the build dir
   # Note: This gives us a type of "developer" installation
   if opt_env['prefix'] == unspecified_prefix:
      if hasattr(os,'symlink'):
         opt_env['INSTALL'] = sca_util.symlinkInstallFunc
      opt_env['prefix'] = pj( Dir('.').get_abspath(), base_build_dir, 'instlinks')

   if opt_env['versioning']:
      version_dot_suffix = ''
      versioned_include_dir = ''
      # We do not use a versioned header directory on Windows.
      if not sys.platform.startswith('win'):
         version_dot_suffix = "-%s.%s.%s" % VRKIT_VERSION
         versioned_include_dir = "vrkit%s" % version_dot_suffix
      version_suffix = "-%s_%s_%s" % VRKIT_VERSION
   else:
      version_suffix = ''
      version_dot_suffix = ''
      versioned_include_dir = ''

   # Setup a map with the paths that we will install everything to
   # - Note: this is used throughout the build to keep things consistent
   inst_paths = {}
   inst_paths['base'] = opt_env['prefix']
   inst_paths['bin'] = pj(inst_paths['base'], 'bin')
   inst_paths['lib'] = pj(inst_paths['base'],'lib')
   inst_paths['lib_plugin'] = pj(inst_paths['lib'], 'vrkit%s' % version_dot_suffix, 'plugins')
   inst_paths['include'] = os.path.normpath(pj(inst_paths['base'], 'include',
                                               versioned_include_dir))
   inst_paths['share'] = pj(inst_paths['base'], 'share', 'vrkit%s' % version_dot_suffix)
   inst_paths['definitions'] = pj(inst_paths['share'],'definitions')
   inst_paths['app_base'] = pj(inst_paths['share'],'apps')
   inst_paths['test_base'] = pj(inst_paths['share'],'test')

   print "Using prefix: ", inst_paths['base']
   print "types: ",    variant_helper.variants["type"] 
   print "archs: ",    variant_helper.variants["arch"] 

   # XXX: Hack around SConsAddons.Options.OpenSG.OpenSG not letting us provide
   # extra arguments to osg-config.  (Of course, if osg-config wasn't totally
   # braindead on Mac OS X, this wouldn't be such a problem.)
   if GetPlatform() == 'darwin':
      mac_opts = os.popen(opensg_options.osgconfig_cmd + " --cflags System").read()
      opensg_options.found_cflags = mac_opts.strip().split(" ")

   # Setup the build environment
   # Add local directory to path and the "root" directory for this project
   vrj_options.apply(opt_env)
   #vrj_options.dumpSettings()

   # Incorporate the Boost options into opt_env if Boost is available.  This
   # accounts for the case when a Boost installation is not available through
   # the output returned by 'vrjuggler-config --cxxflags'.
   if boost_options.isAvailable():
      boost_options.apply(opt_env)
   else:
      print "WARNING: Cannot build vrkit without Boost.Signals"

   if "win32" == platform:
      opt_env.Append(CPPFLAGS = ["/wd4103", "/wd4251", "/wd4275"])

   # ----- FOR EACH VARIANT ----- #
   for combo in variant_helper.iterate(locals(), base_bldr, opt_env):
      opensg_options.apply(build_env, optimize = combo['type'] != 'debugrt')

      vrkit_cxxflags = ''
      # If we are debug or debug runtime, then we define preprocessor symbols
      # to indicate as much.
      if combo['type'].startswith('debug'):
         build_env.AppendUnique(CPPDEFINES = ['VRKIT_DEBUG', 'JUGGLER_DEBUG'])

      # If we are optimized we want no debugging at all.
      elif combo['type'] == 'optimized':
         build_env.AppendUnique(CPPDEFINES = ['NDEBUG', 'VRKIT_OPT',
                                              'JUGGLER_OPT'])
      if opt_env['versioning']:
         build_env.AppendUnique(CPPDEFINES = ['VRKIT_USE_VERSIONING'])

      # Enable boost auto-linking.
      if platform == 'win32':
         build_env.Append(CPPDEFINES = ['BOOST_ALL_DYN_LINK'])

#      if build_env.has_key('MSVS'):
#         import pprint
#         print "Found MSVS. using version: ", build_env['MSVS']['VERSION']
#         pprint.pprint(build_env['MSVS'])

      # Determine the build dir for this variant
      full_build_dir = pj(base_build_dir, combo_dir)
      print "Build Directory: ", full_build_dir
      print "Using prefix: ", opt_env["prefix"]

      if platform == "win32":
         build_env.Append(LINKFLAGS = ['/OPT:NOREF'])

      # Set the directory to install libraries into.
      inst_paths["lib"] = os.path.normpath(pj(inst_paths["base"], "lib",
                                              lib_subdir))

      # Specify the OpenGL libraries.
      if platform == "win32":   
         build_env.Append(CPPFLAGS = ["/wd4996"])
         gl_libraries = ["opengl32"]
      else:
         gl_libraries = ["GL"]

      if GetPlatform() == "win32":
         inst_paths["include_path_flag"] = "/I"
         inst_paths["lib_path_flag"]     = "/LIBPATH:"
      else:
         inst_paths["include_path_flag"] = "-I"
         inst_paths["lib_path_flag"]     = "-L"

      # Don't put a libname to link against on Windows because
      # of automatic linking
      vrkit_lib = ""
      if platform != "win32":
         vrkit_lig = "-lvrkit" + version_suffix

      inst_paths['flagpoll'] = pj(inst_paths['lib'], 'flagpoll')

      # Build up substitution map
      submap = {
         '@prefix@'                    : inst_paths['base'],
         '@exec_prefix@'               : '${prefix}',
         '@vrkit_cxxflags@'            : vrkit_cxxflags,
         '@include_path_flag@'         : inst_paths['include_path_flag'],
         '@includedir@'                : inst_paths['include'],
         '@share_dir@'                 : inst_paths['share'],
         '@vrkit_libs@'                : vrkit_lib,
         '@lib_path_flag@'             : inst_paths['lib_path_flag'],
         '@libdir@'                    : inst_paths['lib'],
         '@arch@'                      : combo['arch'],
         '@version@'                   : vrkit_version_str
      }

      # Setup builder for flagpoll files
      name_parts = ['vrkit',vrkit_version_str,combo['arch']]
      if combo["type"] != "optimized":
         name_parts.append(combo["type"])
      fpc_filename = "-".join(name_parts) + ".fpc"
      vrkit_fpc   = build_env.SubstBuilder(pj(inst_paths['flagpoll'],
                                              fpc_filename),
                                           'vrkit.fpc.in', submap = submap)
      build_env.AddPostAction(vrkit_fpc, Chmod('$TARGET', 0644))
      build_env.Depends(vrkit_fpc, 'src/vrkit/Version.h')

      Export('build_env','full_build_dir', 'combo', 'version_suffix', 'shared_lib_suffix', 'runtime_suffix', 'inst_paths', 'variant_pass')
      SConscript(dirs=['src'], build_dir=full_build_dir, duplicate=0)

   # Get the source location of data files.
   srcdir_abs_path = opt_env.Dir('data').srcnode().abspath
   #print "src dir abs: ", srcdir_abs_path
   data_files = []

   # Collect all file arguments from the walk into dirs and files
   def collect_data_files(junk, dirPath, files):
      for n in files:
         if os.path.isfile(os.path.join(dirPath, n)):
            full_filename = pj(dirPath, n)
            if 0 == full_filename.count('.svn'):
               data_files.append(full_filename)
   os.path.walk(srcdir_abs_path, collect_data_files, None)

   # Find the part of the source path to strip off.
   strip_path = srcdir_abs_path
   if not strip_path.endswith(os.sep):
      strip_path += os.sep

   # Install the contents of the data directory
   for fname in data_files:
      base_fname = fname.replace(strip_path, '')
      opt_env.InstallAs(pj(inst_paths['share'], 'data', base_fname), fname)

   # Aliases
   opt_env.Alias('install', inst_paths['base'])

Help(help_text)
