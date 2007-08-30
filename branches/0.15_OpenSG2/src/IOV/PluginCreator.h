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

#ifndef _INF_PLUGIN_CREATOR_H_
#define _INF_PLUGIN_CREATOR_H_

#include <IOV/Config.h>

#include <string>
#include <boost/function.hpp>
#include <vpr/vpr.h>

#include <IOV/PluginPtr.h>


namespace inf
{

class IOV_CLASS_API PluginCreator
{
public:
   PluginCreator(boost::function<inf::PluginPtr ()> creator,
                 const std::string& pluginName,
                 const vpr::Uint32 pluginMajorVer = 1,
                 const vpr::Uint32 pluginMinorVer = 0,
                 const vpr::Uint32 pluginPatchVer = 0);

   ~PluginCreator();

   inf::PluginPtr createPlugin() const
   {
      return mCreator();
   }

   const std::string& getPluginName() const
   {
      return mPluginName;
   }

   vpr::Uint32 getPluginMajorVersion() const
   {
      return mPluginMajorVer;
   }

   vpr::Uint32 getPluginMinorVersion() const
   {
      return mPluginMajorVer;
   }

   vpr::Uint32 getPluginPatchVersion() const
   {
      return mPluginPatchVer;
   }

   const std::string& getPluginVersion() const
   {
      return mPluginVersionStr;
   }

private:
   boost::function<inf::PluginPtr ()> mCreator;

   std::string mPluginName;

   vpr::Uint32 mPluginMajorVer;
   vpr::Uint32 mPluginMinorVer;
   vpr::Uint32 mPluginPatchVer;
   std::string mPluginVersionStr;
};

}


#endif
