#ifndef _INF_MODE_SWITCH_PLUGIN_H_
#define _INF_MODE_SWITCH_PLUGIN_H_

#include <OpenSG/VRJ/Viewer/plugins/PluginConfig.h>

#include <string>
#include <vector>
#include <boost/enable_shared_from_this.hpp>

#include <vpr/vpr.h>
#include <vpr/DynLoad/Library.h>

#include <OpenSG/VRJ/Viewer/IOV/Plugin.h>
#include <OpenSG/VRJ/Viewer/IOV/WandInterfacePtr.h>
#include <OpenSG/VRJ/Viewer/plugins/ModeSwitchPluginPtr.h>


namespace inf
{

class ModeSwitchPlugin
   : public inf::Plugin
   , public boost::enable_shared_from_this<ModeSwitchPlugin>
{
public:
   static ModeSwitchPluginPtr create()
   {
      return ModeSwitchPluginPtr(new ModeSwitchPlugin());
   }

   virtual ~ModeSwitchPlugin()
   {
      /* Do nothing. */ ;
   }

   virtual std::string getDescription();

   virtual void init(inf::ViewerPtr viewer);

   virtual bool canHandleElement(jccl::ConfigElementPtr elt);

   virtual bool config(jccl::ConfigElementPtr elt);

   virtual void update(inf::ViewerPtr viewer);

   void addPlugin(inf::PluginPtr plugin);

protected:
   ModeSwitchPlugin()
      : SWITCH_BUTTON(3)
      , mCurrentMode(0)
   {
      /* Do nothing. */ ;
   }

   static std::string getElementType()
   {
      return std::string("mode_switch_plugin");
   }

   inf::ViewerPtr mViewer;

   WandInterfacePtr mWandInterface;
   const int SWITCH_BUTTON;

   std::vector<vpr::LibraryPtr> mLoadedDsos;

   int mCurrentMode;
   std::vector<inf::PluginPtr> mPlugins;
};

}


#endif
