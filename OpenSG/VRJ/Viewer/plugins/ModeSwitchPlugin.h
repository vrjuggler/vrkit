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
#include <OpenSG/VRJ/Viewer/IOV/PluginFactoryPtr.h>
#include <OpenSG/VRJ/Viewer/plugins/Buttons.h>
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

   virtual void updateState(inf::ViewerPtr viewer);

   virtual void run(inf::ViewerPtr viewer);

   /**
    * Invokes the global scope delete operator.  This is required for proper
    * releasing of memory in DLLs on Win32.
    */
   void operator delete(void* p)
   {
      ::operator delete(p);
   }

protected:
   /**
    * Deletes this object.  This is an implementation of the pure virtual
    * inf::Plugin::destroy() method.
    */
   virtual void destroy()
   {
      delete this;
   }

   ModeSwitchPlugin()
      : SWITCH_BUTTON(inf::buttons::MODE_SWITCH_BUTTON)
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

   inf::PluginFactoryPtr mPluginFactory;

   int mCurrentMode;
   std::vector<inf::PluginPtr> mPlugins;
};

}


#endif
