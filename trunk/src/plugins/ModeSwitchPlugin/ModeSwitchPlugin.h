// vrkit is (C) Copyright 2005-2008
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

#ifndef _VRKIT_MODE_SWITCH_PLUGIN_H_
#define _VRKIT_MODE_SWITCH_PLUGIN_H_

#include <vrkit/plugin/Config.h>

#include <string>
#include <vector>
#include <boost/enable_shared_from_this.hpp>

#include <vpr/vpr.h>
#include <vpr/DynLoad/Library.h>

#include <vrkit/WandInterfacePtr.h>
#include <vrkit/util/DigitalCommand.h>
#include <vrkit/viewer/Plugin.h>


namespace vrkit
{

/**
 * Mode switching plug-in. This vrkit viewer plug-in manages a set of other
 * viewer plug-ins and switches among them when the user changes "modes" in
 * the application.
 */
class ModeSwitchPlugin
   : public viewer::Plugin
   , public boost::enable_shared_from_this<ModeSwitchPlugin>
{
protected:
   ModeSwitchPlugin(const plugin::Info& info)
      : viewer::Plugin(info)
      , mCurrentMode(0)
   {
      /* Do nothing. */ ;
   }

public:
   static viewer::PluginPtr create(const plugin::Info& info)
   {
      return viewer::PluginPtr(new ModeSwitchPlugin(info));
   }

   virtual ~ModeSwitchPlugin()
   {
      /* Do nothing. */ ;
   }

   virtual std::string getDescription();

   /**
    * Initialize and configure this plug-in. This loads the needed support
    * plug-ins and switches to the first mode.
    */
   virtual viewer::PluginPtr init(ViewerPtr viewer);

   virtual void contextInit(ViewerPtr viewer);

   virtual void update(ViewerPtr viewer);

   virtual void contextPreDraw(ViewerPtr viewer);

   virtual void draw(ViewerPtr viewer);

   virtual void contextPostDraw(ViewerPtr viewer);

   virtual void contextClose(ViewerPtr viewer);

private:
   void registerModule(vpr::LibraryPtr module, ViewerPtr viewer);

   /**
    * Internal helper for mode switching.
    * Switch to the given mode. Activates and deactivates the plug-ins needed.
    */
   void switchToMode(const int unsigned modeNum, ViewerPtr viewer);

   static std::string getElementType()
   {
      return std::string("mode_switch_plugin");
   }

   struct PluginData
   {
      std::string               mName;         /**< Configured name of the plug-in. */
      viewer::PluginPtr         mPlugin;       /**< The plug-in. */
      std::vector<unsigned int> mActiveModes;  /**< Indexes of modes where plug-in is active. */
   };

   WandInterfacePtr     mWandInterface;
   util::DigitalCommand mSwitchButton;

   std::vector<std::string>     mModeNames;       /**< The names of the plug-in modes. */
   unsigned                     mCurrentMode;     /**< Current active plug-in. */
   unsigned                     mMaxMode;         /**< The maximum valid mode number. */
   std::vector<PluginData>      mPlugins;
};

}


#endif /* _VRKIT_MODE_SWITCH_PLUGIN_H_ */
