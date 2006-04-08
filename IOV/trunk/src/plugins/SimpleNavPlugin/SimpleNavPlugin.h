// Copyright (C) Infiscape Corporation 2005-2006

#ifndef SIMPLE_NAV_PLUGIN_H
#define SIMPLE_NAV_PLUGIN_H

#include <IOV/Plugin/PluginConfig.h>

#include <boost/enable_shared_from_this.hpp>

#include <IOV/WandInterfacePtr.h>
#include <IOV/Plugin.h>
#include <IOV/Plugin/NavPlugin.h>


namespace inf
{

class SimpleNavPlugin
   : public NavPlugin
   , public boost::enable_shared_from_this<SimpleNavPlugin>
{
public:
   static inf::PluginPtr create()
   {
      inf::PluginPtr new_strategy = inf::PluginPtr(new SimpleNavPlugin());
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

   virtual PluginPtr init(ViewerPtr viewer);

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

   virtual void focusChanged(inf::ViewerPtr viewer);

   virtual void updateNavState(ViewerPtr viewer, ViewPlatform& viewPlatform);

   virtual void runNav(ViewerPtr viewer, ViewPlatform& viewPlatform);

   static std::string getElementType()
   {
      return std::string("simple_nav_plugin");
   }

   WandInterfacePtr mWandInterface;

   bool mCanNavigate;
   NavState mNavState;

   float mVelocity;
   NavMode mNavMode;

   int mForBtn;      /**< Button for forward motion. */
   int mRevBtn;      /**< Button for reverse. */
   int mRotateBtn;   /**< Button for rotate. */
   int mModeBtn;     /**< Button for swapping mode. */

   std::string mForwardText;
   std::string mReverseText;
   std::string mRotateText;
   std::string mModeText;
};

}


#endif
