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

   enum NavState
   {
      RESET,
      TRANSLATE,
      ROTATE
   };

   /** Navigation mode. */
   enum NavMode
   {
      WALK,     /**< Walk (drive) mode */
      FLY       /**< Fly mode */
   };

   SimpleNavPlugin();

   virtual void focusChanged();

   virtual void updateNavState(ViewerPtr viewer, ViewPlatform& viewPlatform);

   virtual void runNav(ViewerPtr viewer, ViewPlatform& viewPlatform);

   WandInterfacePtr mWandInterface;

   bool mCanNavigate;
   NavState mNavState;

   float mVelocity;
   NavMode mNavMode;

   const int ACCEL_BUTTON;
   const int ROTATE_BUTTON;
   const int MODE_BUTTON;
};

}


#endif
