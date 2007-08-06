// Copyright (C) Infiscape Corporation 2005-2007

#ifndef SIMPLE_NAV_PLUGIN_H
#define SIMPLE_NAV_PLUGIN_H

#include <IOV/Plugin/PluginConfig.h>

#include <boost/enable_shared_from_this.hpp>

#include <IOV/WandInterfacePtr.h>
#include <IOV/Plugin.h>
#include <IOV/Plugin/NavPlugin.h>
#include <IOV/Util/DigitalCommand.h>


namespace inf
{

class SimpleNavPlugin
   : public NavPlugin
   , public boost::enable_shared_from_this<SimpleNavPlugin>
{
protected:
   SimpleNavPlugin(const inf::plugin::Info& info);

public:
   static inf::PluginPtr create(const inf::plugin::Info& info)
   {
      return inf::PluginPtr(new SimpleNavPlugin(info));
   }

   virtual ~SimpleNavPlugin()
   {
      ;
   }

   virtual std::string getDescription()
   {
      return std::string("Navigation");
   }

   virtual PluginPtr init(ViewerPtr viewer);

protected:
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

   virtual void focusChanged(inf::ViewerPtr viewer);

   virtual void updateNav(ViewerPtr viewer, ViewPlatform& viewPlatform);

   static std::string getElementType()
   {
      return std::string("simple_nav_plugin");
   }

   WandInterfacePtr mWandInterface;

   bool mCanNavigate;

   float mVelocity;
   NavMode mNavMode;

   inf::DigitalCommand mForBtn;         /**< Button for forward motion. */
   inf::DigitalCommand mRevBtn;         /**< Button for reverse. */
   inf::DigitalCommand mRotateBtn;      /**< Button for rotate. */
   inf::DigitalCommand mModeBtn;        /**< Button for swapping mode. */

   std::string mForwardText;
   std::string mReverseText;
   std::string mRotateText;
   std::string mModeText;
};

}


#endif
