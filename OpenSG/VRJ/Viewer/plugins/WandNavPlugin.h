#ifndef WAND_NAV_PLUGIN_H
#define WAND_NAV_PLUGIN_H

#include <OpenSG/VRJ/Viewer/plugins/PluginConfig.h>

#include <boost/enable_shared_from_this.hpp>

#include <OpenSG/VRJ/Viewer/IOV/WandInterfacePtr.h>

#include <vpr/Util/Interval.h>

#include <OpenSG/VRJ/Viewer/plugins/NavPlugin.h>
#include <OpenSG/VRJ/Viewer/plugins/WandNavPluginPtr.h>


namespace inf
{

class WandNavPlugin
   : public NavPlugin
   , public boost::enable_shared_from_this<WandNavPlugin>
{
public:
   static WandNavPluginPtr create()
   {
      WandNavPluginPtr new_strategy = WandNavPluginPtr(new WandNavPlugin);
      return new_strategy;
   }

   virtual ~WandNavPlugin()
   {
      ;
   }

   virtual std::string getDescription()
   {
      return std::string("Navigation");
   }

   virtual void init(ViewerPtr viewer);

   /**
    * @pre \c init() has been called.
    */
   virtual bool canHandleElement(jccl::ConfigElementPtr elt);

   /**
    * @pre \c canHandleElement() returned true.
    */
   virtual bool config(jccl::ConfigElementPtr elt);

   /** @name Configuration methods */
   //@{
   void setMaximumVelocity(const float minVelocity);
   void setAcceleration(const float acceleration);
   //@}

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

   /** Navigation mode. */
   enum NavMode
   {
      WALK,     /**< Walk (drive) mode */
      FLY       /**< Fly mode */
   };

   WandNavPlugin()
      : mLastFrameTime(0, vpr::Interval::Sec)
      , mVelocity(0.0f)
      , mMaxVelocity(0.5f)
      , mAcceleration(0.005f)
      , mNavMode(WALK)
      , ACCEL_BUTTON(0)
      , STOP_BUTTON(1)
      , ROTATE_BUTTON(2)
      , MODE_BUTTON(3)
   {
      ;
   }

   virtual void updateNav(ViewerPtr viewer, ViewPlatform& viewPlatform);

   static std::string getElementType()
   {
      return std::string("wand_nav_plugin");
   }

   vpr::Interval mLastFrameTime;

   WandInterfacePtr mWandInterface;

   float mVelocity;
   float mMaxVelocity;
   float mAcceleration;
   NavMode mNavMode;

   const int ACCEL_BUTTON;
   const int STOP_BUTTON;
   const int ROTATE_BUTTON;
   const int MODE_BUTTON;
};

}


#endif
