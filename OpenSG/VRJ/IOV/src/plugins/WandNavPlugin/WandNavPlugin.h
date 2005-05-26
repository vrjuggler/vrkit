#ifndef WAND_NAV_PLUGIN_H
#define WAND_NAV_PLUGIN_H

#include <IOV/Plugin/PluginConfig.h>

#include <boost/enable_shared_from_this.hpp>

#include <vpr/Util/Interval.h>

#include <IOV/WandInterfacePtr.h>
#include <IOV/Plugin/NavPlugin.h>


namespace inf
{

class WandNavPlugin
   : public NavPlugin
   , public boost::enable_shared_from_this<WandNavPlugin>
{
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

   virtual void init(ViewerPtr viewer);

   /**
    * @pre \c canHandleElement() returned true.
    */
   bool config(jccl::ConfigElementPtr elt);

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

   WandNavPlugin();

   virtual void focusChanged();

   virtual void updateNavState(ViewerPtr viewer, ViewPlatform& viewPlatform);

   virtual void runNav(ViewerPtr viewer, ViewPlatform& viewPlatform);

   static std::string getElementType()
   {
      return std::string("wand_nav_plugin");
   }

   vpr::Interval mLastFrameTime;

   WandInterfacePtr mWandInterface;

   bool mCanNavigate;
   NavState mNavState;

   float mVelocity;
   float mMaxVelocity;
   float mAcceleration;
   NavMode mNavMode;

   int mForBtn;      /**< Button for forward motion. */
   int mRevBtn;      /**< Button for reverse. */
   int mRotateBtn;   /**< Button for rotate. */
   int mModeBtn;     /**< Button for swapping mode. */
};

}


#endif
