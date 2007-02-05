// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_MODE_SWITCH_PLUGIN_H_
#define _INF_MODE_SWITCH_PLUGIN_H_

#include <IOV/Plugin/PluginConfig.h>

#include <string>
#include <vector>
#include <boost/enable_shared_from_this.hpp>

#include <vpr/vpr.h>
#include <vpr/DynLoad/Library.h>

#include <IOV/Plugin.h>
#include <IOV/WandInterfacePtr.h>
#include <IOV/Util/DigitalCommand.h>


namespace inf
{

/** Mode switching plugin.
 *
 * This plugin manages a set of other plugins and switches among them
 * when the user changes "modes" in the application.
 */
class ModeSwitchPlugin
   : public inf::Plugin
   , public boost::enable_shared_from_this<ModeSwitchPlugin>
{
protected:
   ModeSwitchPlugin()
      : mCurrentMode(0)
   {
      /* Do nothing. */ ;
   }

public:
   static inf::PluginPtr create()
   {
      return inf::PluginPtr(new ModeSwitchPlugin());
   }

   virtual ~ModeSwitchPlugin()
   {
      /* Do nothing. */ ;
   }

   virtual std::string getDescription();

   /** Initialize and configure the plugin.
    * This loads the needed support plugins
    * and switches to the first mode.
    */
   virtual PluginPtr init(inf::ViewerPtr viewer);

   virtual void contextInit(inf::ViewerPtr viewer);

   virtual void update(inf::ViewerPtr viewer);

   virtual void contextPreDraw(inf::ViewerPtr viewer);

   virtual void draw(inf::ViewerPtr viewer);

   virtual void contextPostDraw(inf::ViewerPtr viewer);

   virtual void contextClose(inf::ViewerPtr viewer);

   /**
    * Invokes the global scope delete operator.  This is required for proper
    * releasing of memory in DLLs on Win32.
    */
   void operator delete(void* p)
   {
      ::operator delete(p);
   }

protected:

   /** Internal helper for mode switching.
   * Switch to the given mode. Activates and deactivates the plugins needed.
   */
   void switchToMode(const int unsigned modeNum, inf::ViewerPtr viewer);

   /**
    * Deletes this object.  This is an implementation of the pure virtual
    * inf::Plugin::destroy() method.
    */
   virtual void destroy()
   {
      delete this;
   }

   static std::string getElementType()
   {
      return std::string("mode_switch_plugin");
   }

protected:
   struct PluginData
   {
      std::string            mName;         /**< Configured name of the plugin. */
      inf::PluginPtr         mPlugin;       /**< The plugin. */
      std::vector<unsigned>  mActiveModes;  /**< Indexes of modes where plugin is active. */
   };

   WandInterfacePtr    mWandInterface;
   inf::DigitalCommand mSwitchButton;

   std::vector<std::string>     mModeNames;       /**< The names of the plugin modes. */
   unsigned                     mCurrentMode;     /**< Current active plugin. */
   unsigned                     mMaxMode;         /**< The maximum valid mode number. */
   std::vector<PluginData>      mPlugins;
};

}


#endif
