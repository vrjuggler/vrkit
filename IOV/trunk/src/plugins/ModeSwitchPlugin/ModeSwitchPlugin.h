// vrkit is (C) Copyright 2005-2007
//    by Allen Bierbaum, Aron Bierbuam, Patrick Hartling, and Daniel Shipton
//
// This file is part of vrkit.
//
// vrkit is free software; you can redistribute it and/or modify it under the
// terms of the GNU Lesser General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// vrkit is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
// more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
   ModeSwitchPlugin(const inf::plugin::Info& info)
      : Plugin(info)
      , mCurrentMode(0)
   {
      /* Do nothing. */ ;
   }

public:
   static inf::PluginPtr create(const inf::plugin::Info& info)
   {
      return inf::PluginPtr(new ModeSwitchPlugin(info));
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

private:
   void registerModule(vpr::LibraryPtr module, inf::ViewerPtr viewer);

   /** Internal helper for mode switching.
   * Switch to the given mode. Activates and deactivates the plugins needed.
   */
   void switchToMode(const int unsigned modeNum, inf::ViewerPtr viewer);

   static std::string getElementType()
   {
      return std::string("mode_switch_plugin");
   }

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
