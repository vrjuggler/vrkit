"""SConsAddons.Options.FlagPollBasedOption

Defines common options structure for Options that use flagpoll.
"""

#
# __COPYRIGHT__
#
# This file is part of scons-addons.
#
# Scons-addons is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# Scons-addons is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with scons-addons; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

__revision__ = "__FILE__ __REVISION__ __DATE__ __DEVELOPER__"

import SCons.Environment     # Get the environment stuff
import SCons
import SConsAddons.Options   # Get the modular options stuff
import SCons.Util
import sys, os, re, string
import SConsAddons.Util as sca_util

from SCons.Util import WhereIs
pj = os.path.join


class FlagPollBasedOption(SConsAddons.Options.PackageOption):
   """ 
   Options object for capturing common options and deps for flagpoll based options
   """

   def __init__(self, name, moduleName, requiredVersion, required, useCppPath, helpText,):
      """
         name - The name to use for this option
         moduleName - The name of the module to look for.
         requiredVersion - The version of project required (ex: "0.16.7")
         required - Is the dependency required?  (if so we exit on errors)
         useCppPath - If true, put the include paths in cpppath else, put them in cxxflags.
         helpText - The help text to use for it all.
      """
      # Common settings needed for all tools (these must be setup in derived classes
      required_attrs = ['baseDirKey',                        
                        'filesToCheckRelBase']
      for a in required_attrs:
         assert hasattr(self,a), "FlagPollBasedOption needs attribute [%s] set before calling its init."%a
         
      #self.baseDirKey = "NoneBaseDir"
      #self.optionName = "No option name"
      #self.configCmdName = 'vrj-config'
      #self.filesToCheckRelBase = [pj('include','vpr','vprConfig.h'), ... ]
      
      SConsAddons.Options.PackageOption.__init__(self, name, self.baseDirKey, helpText);
            
      # Local options
      self.baseDir = None
      self.moduleName = moduleName      
      self.requiredVersion = requiredVersion
      self.required = required
      self.useCppPath = useCppPath      
   
      self.flagpoll_parser = sca_util.FlagPollParser(self.moduleName)
      if not self.flagpoll_parser.valid:
         print "Option: %s  Could not init flagpoll parser."%self.moduleName
   
   def startProcess(self):
      print "Checking for %s..."%self.moduleName,
      
   def setInitial(self, optDict):
      " Set initial values from given dict "
      if self.verbose:
         print "   Setting initial %s settings."%self.moduleName
      if optDict.has_key(self.baseDirKey):
         self.baseDir = optDict[self.baseDirKey]
         if self.verbose:
            print "   %s specified or cached. [%s]."% (self.baseDirKey,self.baseDir)
        
   def find(self, env):
      # Quick exit if nothing to find
      if (self.baseDir != None):
         return
      
      # Call flagpoll for information
      # Find cmd-config and call it to get the other arguments
      print "   searching...",
      if self.flagpoll_parser.valid:
         print "[found]"
      else:
         print "[failed]"      

   def validate(self, env):
      # Check that path exist
      # Check that cmd-config exist
      # Check that an include file: include/vpr/vprConfig.h  exists
      # Update the temps for later usage
      passed = True
      
      self.found_incs = None
      self.found_libs = None
      self.found_lib_paths = None
      self.found_link_from_libs = None
      
      # Try using basedir
      if self.baseDir:      
         if not os.path.isdir(self.baseDir):
            passed = False
            self.checkRequired("vrj base dir does not exist:%s"%self.baseDir)
      
         found_ver_str = "unknown"
         print "Skipping version check..."
         
         # Set of files to check for
         for f in self.filesToCheckRelBase:
            check_file = pj(self.baseDir, f)
            if not os.path.isfile(check_file):
               passed = False
               self.checkRequired("%s not found:%s"%(f,check_file))

         if passed:
            # Just guess
            self.found_incs = [pj(self.baseDir,'include'),]
            self.found_libs = []
            self.found_lib_paths = [pj(self.baseDir,'lib'),]
      # Use flagpoll
      else:
         found_ver_str = self.flagpoll_parser.getVersion()
         if not self.flagpoll_parser.valid:
            self.checkRequired("   version call failed: %s"%self.flagpoll_parser.flagpoll_cmd)
            passed = False
         else:            
            req_ver = [int(n) for n in self.requiredVersion.split(".")]
            found_ver = [int(n) for n in found_ver_str.split(".")]
            if found_ver < req_ver:
               passed = False
               self.checkRequired("   found version is to old: required:%s found:%s"%
                                  (self.requiredVersion,found_ver_str))
      
         if passed:                              
            # Returns lists of the options we want
            self.found_incs           = self.flagpoll_parser.findIncludes()
            self.found_libs           = self.flagpoll_parser.findLibs()
            self.found_lib_paths      = self.flagpoll_parser.findLibPaths()
            self.found_link_from_libs = self.flagpoll_parser.findLinkFlags()      

      #print "-----------------------"
      #print "self.found_libs:", self.found_libs
      #print "self.found_lib_paths:", self.found_lib_paths
      #print "self.found_link_from_libs:", self.found_link_from_libs

      if not passed:
         # Clear everything and remove the key from the environment
         self.baseDir = None         
         edict = env.Dictionary()
         if edict.has_key(self.baseDirKey):
            del edict[self.baseDirKey]
            
      # Create list of flags that may be needed later
      self.found_incs_as_flags = [env["INCPREFIX"] + p for p in self.found_incs];
      self.available = True
      print "   %s version: %s [OK]" % (self.moduleName,found_ver_str)

      return passed


   def apply(self, env, useCppPath=False):
      """ Add environment options for building against vrj-based library"""
      if self.found_incs:
         if self.useCppPath or useCppPath:
            env.AppendUnique(CPPPATH = self.found_incs)
         else:
            env.AppendUnique(CXXFLAGS = self.found_incs_as_flags)
      if self.found_libs:
         env.AppendUnique(LIBS = self.found_libs)
      if self.found_lib_paths:
         env.AppendUnique(LIBPATH = self.found_lib_paths)
      if self.found_link_from_libs:
         env.AppendUnique(LINKFLAGS = self.found_link_from_libs)
   
   def getSettings(self):
      return [(self.baseDirKey, self.baseDir),]
   
   def dumpSettings(self):
      "Write out the settings"
      print "%s: %s", (self.baseDirKey, self.baseDir)
      print "%s: %s"%(self.configCmdName,self.configCmdFullPath)
      print "CPPPATH:", self.found_incs
      print "CPPPATH as flags:", self.found_incs_as_flags
      print "LIBS:", self.found_libs
      print "LIBPATH:", self.found_lib_paths
      print "LINKFLAGS:", self.found_link_from_libs
