#python

import getopt
import glob
import os
import re
import shutil
import sys
import traceback

EXIT_STATUS_SUCCESS           = 0
EXIT_STATUS_NO_MSVS           = 1
EXIT_STATUS_MISSING_DATA_FILE = 2
EXIT_STATUS_MSVS_START_ERROR  = 3
EXIT_STATUS_INVALID_PATH      = 4
EXIT_STATUS_MISSING_REQ_VALUE = 5

gProjectDir      = os.path.dirname(os.path.abspath(sys.argv[0]))
gOptionsFileName = "options.cache"

class BuildOption:
   def __init__(self, envVar, desc, defaultValue, isDirectory = True,
                required = True):
      self.envVar      = envVar
      self.desc        = desc
      self.default     = defaultValue
      self.required    = required
      self.isDirectory = isDirectory

def noVisualStudioError():
   print "ERROR: Visual Studio commands are not in your path!"
   print "Run vsvars32.bat in this shell or update the %PATH% environment"
   print "variable on your system."
   sys.exit(EXIT_STATUS_NO_MSVS)

def guessBoostToolset(reattempt = False):
   (cl_stdin, cl_stdout, cl_stderr) = os.popen3('cl')
   cl_version_line = cl_stderr.readline()

   cl_ver_match = re.compile(r'Compiler Version ((\d+)\.(\d+)\.(\d+))')
   ver_string_match = cl_ver_match.search(cl_version_line)

   if ver_string_match is not None:
      cl_major = int(ver_string_match.group(2))
      cl_minor = int(ver_string_match.group(3))

      if cl_major == 13 and cl_minor == 0:
         boost_tool_guess = 'vc7'
      elif cl_major == 13 and cl_minor == 10:
         boost_tool_guess = 'vc71'
      else:
         boost_tool_guess = 'vc80'
   else:
      boost_tool_guess = ''

   in_status  = cl_stdin.close()
   out_status = cl_stdout.close()
   err_status = cl_stderr.close()

   if in_status is not None or out_status is not None or err_status is not None:
      if not reattempt:
         vs_dirs = [r'C:\Program Files\Microsoft Visual Studio .NET 2003',
                    r'C:\Program Files\Microsoft Visual Studio 8'
                   ]

         for d in vs_dirs:
            if os.path.exists(d):
               print "NOTE: Using Visual Studio installation in"
               print "      " + d
               vs_path = [os.path.join(d, r'Common7\IDE'),
                          os.path.join(d, r'VC7\BIN'),
                          os.path.join(d, r'VC\BIN'),
                          os.path.join(d, r'Common7\Tools'),
                          os.path.join(d, r'Common7\Tools\bin\prerelease'),
                          os.path.join(d, r'Common7\Tools\bin')]

               path_add = os.pathsep.join(vs_path)
               os.environ['PATH'] = path_add + os.pathsep + \
                                       os.getenv('PATH', '')
               return guessBoostToolset(True)

         noVisualStudioError()

      else:
         noVisualStudioError()

   return boost_tool_guess

def getCacheFileName():
   return os.path.join(gProjectDir, gOptionsFileName)

def processInput(optionDict, envVar, inputDesc, required = False):
   default_value = optionDict[envVar]
   print "  %s [%s]: " % (inputDesc, default_value),
   input_str = sys.stdin.readline().strip(" \n")

   if input_str == '':
      if required and (default_value is None or default_value == ''):
         print "ERROR: %s value required" % inputDesc
         sys.exit(EXIT_STATUS_MISSING_REQ_VALUE)
      else:
         value_str = default_value
   else:
      value_str = input_str

   optionDict[envVar] = value_str
   os.environ[envVar] = value_str

   return value_str

def getDefaultVars():
   boost_tool_fallback = guessBoostToolset()

   required = []
   required.append(BuildOption('BOOST_ROOT',
                               'Boost C++ installation directory', ''))
   required.append(BuildOption('BOOST_VERSION', 'Boost C++ version',
                               '1_32', False))
   required.append(BuildOption('BOOST_INCLUDES',
                               'Directory containing the Boost C++ header tree',
                               ''))
   required.append(BuildOption('BOOST_TOOL',
                               'The Boost.Build toolset library name component',
                               boost_tool_fallback, False))
   required.append(BuildOption('VJ_BASE_DIR',
                               'VR Juggler installation directory', ''))
   required.append(BuildOption('VJ_DEPS_DIR',
                               'VR Juggler dependency installation directory',
                               ''))
   required.append(BuildOption('OSGROOT', 'OpenSG installation directory', ''))

   optional = []

   options = {
      'prefix' : r'build.win32\instlinks',
   }

   for opt in required + optional:
      options[opt.envVar] = os.getenv(opt.envVar, opt.default)

   # If there are cached options, read them in.
   cache_file = getCacheFileName()
   if os.path.exists(cache_file):
      execfile(cache_file)

   return required, optional, options

def setVars():
   required, optional, options = getDefaultVars()

   print "+++ Required Settings"
   processInput(options, 'prefix', 'Installation prefix')

   boost_dir = ''
   boost_ver = ''
   for opt in required:
      result = processInput(options, opt.envVar, opt.desc, opt.required)

      # The following is a little hack to get a reasonable default set for
      # the BOOST_INCLUDES variable before the user has to enter it manually.
      if opt.envVar == 'BOOST_ROOT':
         boost_dir = result
      elif opt.envVar == 'BOOST_VERSION':
         boost_ver = result
         options['BOOST_INCLUDES'] = boost_dir + r'\include\boost-' + boost_ver

   print "+++ Optional Settings"

   for opt in optional:
      processInput(options, opt.envVar, opt.desc, opt.required)

   postProcessOptions(options)
   writeCacheFile(options)

   return options

def postProcessOptions(options):
   # Check for Boost 1.32 Visual C++ toolset names.
   match = re.compile(r'vc-(\d)_(\d)').match(options['BOOST_TOOL'])

   if match is not None:
      os.environ['BOOST_TOOL'] = 'vc%s%s' % (match.group(1), match.group(2))

def writeCacheFile(optionDict):
   cache_file = open(getCacheFileName(), 'w')
   for k, v in optionDict.iteritems():
      output = "options['%s'] = r'%s'\n" % (k, v)
      cache_file.write(output)
   cache_file.close()

def doInstall(prefix, platform):
   makeTree(prefix)
   installIOV(prefix, platform)

def mkinstalldirs(dir):
   if dir != '' and not os.path.exists(dir):
      (head, tail) = os.path.split(dir)
      mkinstalldirs(head)
      os.mkdir(dir)

def makeTree(prefix):
   mkinstalldirs(os.path.join(prefix, 'bin'))
   mkinstalldirs(os.path.join(prefix, 'include'))
   mkinstalldirs(os.path.join(prefix, 'lib'))
   mkinstalldirs(os.path.join(prefix, 'share'))

def installDir(startDir, destDir, allowedExts = None, disallowedExts = None,
               disallowedFiles = None):
   cwd = os.getcwd()

   if not os.path.isabs(destDir):
      destDir = os.path.abspath(destDir)

   mkinstalldirs(destDir)

   os.chdir(startDir)
   contents = os.listdir(startDir)

   if disallowedExts is None:
      disallowedExts = []

   if disallowedFiles is None:
      disallowedFiles = []

   # Add some extensions that should always be disallowed.  This relieves the
   # caller from having to add these repeatedly.
   disallowedExts.append('.ilk')
   disallowedExts.append('.ncb')
   disallowedExts.append('.pdb')
   disallowedExts.append('.suo')

   skip_dirs = ['.svn', 'autom4te.cache']
   for f in contents:
      if os.path.isdir(f):
         if f in skip_dirs:
            continue

         start_dir = os.path.join(startDir, f)
         dest_dir  = os.path.join(destDir, f)
         installDir(start_dir, dest_dir, allowedExts, disallowedExts,
                    disallowedFiles)
      else:
         (root, f_ext) = os.path.splitext(f)
         if allowedExts is None:
            if f_ext not in disallowedExts:
               shutil.copy2(f, destDir)
         elif f_ext in allowedExts:
            (head, tail) = os.path.split(f)
            if f not in disallowedFiles:
               shutil.copy2(f, destDir)

   os.chdir(cwd)

def installLibs(srcRoot, destdir,
                buildPlatforms = ['Win32', 'x64'],
                buildTypes = ['ReleaseDLL', 'DebugDLL', 'Release', 'Debug'],
                extensions = ['.dll', '.lib']):
   for p in buildPlatforms:
      for t in buildTypes:
         srcdir = os.path.join(srcRoot, p, t)
         if os.path.exists(srcdir):
            installDir(srcdir, destdir, extensions)

def installIOV(prefix, platform):
   destdir = os.path.join(prefix, 'include', 'IOV')
   srcdir  = os.path.join(gProjectDir, 'src', 'IOV')
   installDir(srcdir, destdir, ['.h'])

   destdir = os.path.join(prefix, 'lib')
   srcroot = os.path.join(gProjectDir, 'vc71', 'IOV')
   installLibs(srcroot, destdir, buildPlatforms = [platform])

   share_root = os.path.join(prefix, 'share', 'IOV')
   destdir = os.path.join(share_root, 'data')
   srcdir  = os.path.join(gProjectDir, 'data')
   installDir(srcdir, destdir)

   destdir = os.path.join(share_root, 'definitions')
   srcdir  = os.path.join(gProjectDir, 'src', 'IOV')
   installDir(srcdir, destdir, ['.jdef'])
   jdef_destdir = destdir

   installPlugins(prefix, platform, share_root, jdef_destdir)

def installPlugins(prefix, platform, shareRoot, jdefDir):
   destdir = os.path.join(prefix, 'lib', 'IOV', 'plugins')
   plugin_build_dir = os.path.join(gProjectDir, 'vc71', platform, 'plugins')
   installDir(plugin_build_dir, destdir, ['.dll'])

   plugin_src_glob = os.path.join(gProjectDir, 'src', 'plugins', '*Plugin')
   plugin_src_dirs = glob.glob(plugin_src_glob)

   destdir = jdefDir

   for p in plugin_src_dirs:
      installDir(p, destdir, ['.jdef'])

def getVSCmd():
   devenv_cmd = None
   for p in str.split(os.getenv('PATH', ''), os.pathsep):
      if os.path.exists(os.path.join(p, 'devenv.exe')):
         devenv_cmd = os.path.join(p, 'devenv.exe')
         break

   if devenv_cmd is None:
      # The environment variable %VSINSTALLDIR% is set by vsvars32.bat.
      print "WARNING: Falling back on the use of %VSINSTALLDIR%"
      devenv_cmd = r'%s' % os.path.join(os.getenv('VSINSTALLDIR', ''),
                                        'devenv.exe')

   return devenv_cmd

def main():
   options = setVars()

   try:
      cmd_opts, cmd_args = getopt.getopt(sys.argv[1:], "n",)
   except getopt.GetoptError:
      sys.exit(4)

   skip_vs = False

   for o, a in cmd_opts:
      if o in ("-n",):
         skip_vs = True

   try:
      status = 0

      if not skip_vs:
         devenv_cmd    = getVSCmd()
         solution_file = r'"%s"' % os.path.join(gProjectDir, 'vc71',
                                              'Viewer.sln')
         status = os.spawnl(os.P_WAIT, devenv_cmd, 'devenv', solution_file)

      if status == 0:
         print "Proceed with IOV installation [y]: ",
         proceed = sys.stdin.readline().strip(" \n")

         if proceed == '' or proceed.lower().startswith('y'):
            doInstall(options['prefix'], 'Win32')
   except OSError, osEx:
      print "Could not execute %s: %s" % (devenv_cmd, osEx)
      sys.exit(EXIT_STATUS_MSVS_START_ERROR)

   sys.exit(EXIT_STATUS_SUCCESS)

if __name__ == '__main__':
   try:
      main()
   except SystemExit, exitEx:
      if exitEx.code == EXIT_STATUS_SUCCESS:
         status = 'successful completion'
      elif exitEx.code == EXIT_STATUS_NO_MSVS:
         status = 'no Visual Studio installation found'
      elif exitEx.code == EXIT_STATUS_MISSING_DATA_FILE:
         status = 'could not read data file required for compiling'
      elif exitEx.code == EXIT_STATUS_MSVS_START_ERROR:
         status = 'could not start Visual Studio'
      elif exitEx.code == EXIT_STATUS_INVALID_PATH:
         status = 'invalid directory structure'
      elif exitEx.code == EXIT_STATUS_MISSING_REQ_VALUE:
         status = 'required value not given'
      else:
         status = 'error encountered'

      print "Exiting with status %d (%s)" % (exitEx.code, status)
      print "Press <ENTER> to quit ..."
      sys.stdin.readline()

      # Exit for real without throwing another SystemExit exception.
      os._exit(exitEx.code)
   except:
      info = sys.exc_info()
      traceback.print_exception(info[0], info[1], info[2])
      print "An exception was caught.  Press <ENTER> to quit ..."
      sys.stdin.readline()
