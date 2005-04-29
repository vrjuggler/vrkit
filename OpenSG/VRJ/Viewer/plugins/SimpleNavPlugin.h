#ifndef SIMPLE_NAV_PLUGIN_H
#define SIMPLE_NAV_PLUGIN_H

#include <OpenSG/VRJ/Viewer/plugins/PluginConfig.h>

#include <boost/enable_shared_from_this.hpp>

#include <OpenSG/VRJ/Viewer/IOV/WandInterfacePtr.h>
#include <OpenSG/VRJ/Viewer/IOV/Plugin.h>
#include <OpenSG/VRJ/Viewer/plugins/NavPlugin.h>
#include <OpenSG/VRJ/Viewer/plugins/SimpleNavPluginPtr.h>


namespace inf
{

class SimpleNavPlugin
   : public NavPlugin
   , public boost::enable_shared_from_this<SimpleNavPlugin>
{
public:
   static SimpleNavPluginPtr create()
   {
      SimpleNavPluginPtr new_strategy = SimpleNavPluginPtr(new SimpleNavPlugin);
      return new_strategy;
   }

   virtual ~SimpleNavPlugin()
   {
      ;
   }

   virtual std::string getDescription()
   {
      return std::string("Navigation");
   }

   virtual void init(ViewerPtr viewer);

   virtual bool canHandleElement(jccl::ConfigElementPtr elt)
   {
      return false;
   }

   virtual bool config(jccl::ConfigElementPtr elt)
   {
      return false;
   }

protected:
   /** Navigation mode. */
   enum NavMode
   {
      WALK,     /**< Walk (drive) mode */
      FLY       /**< Fly mode */
   };

   SimpleNavPlugin()
      : mVelocity(0.0f)
      , mNavMode(WALK)
      , ACCEL_BUTTON(0)
      , STOP_BUTTON(1)
      , ROTATE_BUTTON(2)
      , MODE_BUTTON(3)
   {
      ;
   }

   virtual void updateNav(ViewerPtr viewer, ViewPlatform& viewPlatform);

   WandInterfacePtr mWandInterface;

   float mVelocity;
   NavMode mNavMode;

   const int ACCEL_BUTTON;
   const int STOP_BUTTON;
   const int ROTATE_BUTTON;
   const int MODE_BUTTON;
};

}


#endif
