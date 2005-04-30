#ifndef _INF_PLUGIN_CREATOR_H_
#define _INF_PLUGIN_CREATOR_H_

#include <OpenSG/VRJ/Viewer/IOV/Config.h>

#include <string>
#include <vpr/vpr.h>

#include <OpenSG/VRJ/Viewer/IOV/PluginPtr.h>


namespace inf
{

class IOV_CLASS_API PluginCreator
{
public:
   PluginCreator(const std::string& pluginName,
                 const vpr::Uint32 pluginMajorVer = 1,
                 const vpr::Uint32 pluginMinorVer = 0,
                 const vpr::Uint32 pluginPatchVer = 0);

   ~PluginCreator();

   void setPlugin(inf::PluginPtr plugin)
   {
      mPluginPtr = plugin;
   }

   inf::PluginPtr getPlugin() const
   {
      return mPluginPtr;
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
   std::string mPluginName;

   vpr::Uint32 mPluginMajorVer;
   vpr::Uint32 mPluginMinorVer;
   vpr::Uint32 mPluginPatchVer;
   std::string mPluginVersionStr;

   inf::PluginPtr mPluginPtr;
};

}


#endif
