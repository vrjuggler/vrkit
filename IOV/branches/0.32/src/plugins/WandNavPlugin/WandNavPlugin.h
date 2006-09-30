// Copyright (C) Infiscape Corporation 2005-2006

#ifndef WAND_NAV_PLUGIN_H
#define WAND_NAV_PLUGIN_H

#include <IOV/Plugin/PluginConfig.h>

#include <boost/enable_shared_from_this.hpp>

#include <vpr/Util/Interval.h>

#include <IOV/WandInterfacePtr.h>
#include <IOV/Util/DigitalCommand.h>
#include <IOV/Plugin/NavPlugin.h>


namespace inf
{

class WandNavPlugin
   : public NavPlugin
   , public boost::enable_shared_from_this<WandNavPlugin>
{
protected:
   WandNavPlugin();

public:
   static inf::PluginPtr create()
   {
      inf::PluginPtr new_strategy = inf::PluginPtr(new WandNavPlugin());
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

   virtual PluginPtr init(ViewerPtr viewer);

   /**
    * @pre \c canHandleElement() returned true.
    */
   bool config(jccl::ConfigElementPtr elt);

   /** @name Configuration methods */
   //@{
   void setMaximumVelocity(const float minVelocity);
   void setAcceleration(const float acceleration);
   void setDeceleration(const float deceleration);
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

   enum NavState
   {
      NONE,
      RESET,
      TRANSLATE,
      ROTATE
   };

   /** Navigation mode. */
   enum NavMode
   {
      WALK = 0,     /**< Walk (drive) mode */
      FLY  = 1      /**< Fly mode */
   };

   virtual void focusChanged(inf::ViewerPtr viewer);

   virtual void updateNav(ViewerPtr viewer, ViewPlatform& viewPlatform);

   std::vector<int> transformButtonVec(const std::vector<int>& btns);

   static std::string getElementType()
   {
      return std::string("wand_nav_plugin");
   }

   vpr::Interval mLastFrameTime;

   WandInterfacePtr mWandInterface;

   bool mCanNavigate;

   float mVelocity;
   float mMaxVelocity;
   float mAcceleration;
   float mDeceleration;
   bool mIsDecelerationEnabled;
   float mRotationSensitivity;      /**< Scalar on rotation delta.  Used to adjust sensitivity. */
   NavMode mNavMode;

   inf::DigitalCommand mForwardBtn;     /**< Button for forward motion. */
   inf::DigitalCommand mReverseBtn;     /**< Button for reverse. */
   inf::DigitalCommand mRotateBtn;      /**< Button for rotate. */
   inf::DigitalCommand mModeBtn;        /**< Button for swapping mode. */
   inf::DigitalCommand mResetBtn;       /**< Button for resetting location. */

   std::string mForwardText;
   std::string mReverseText;
   std::string mRotateText;
   std::string mModeText;
   std::string mResetText;
};

}


#endif
