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

#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#endif

#include <iostream>
#include <sstream>
#include <algorithm>
#include <boost/bind.hpp>
#include <boost/assign/list_of.hpp>

#include <vpr/vpr.h>
#include <vpr/System.h>
#include <jccl/Config/ConfigElement.h>

#include <vrkit/Scene.h>
#include <vrkit/InterfaceTrader.h>
#include <vrkit/User.h>
#include <vrkit/Viewer.h>
#include <vrkit/WandInterface.h>
#include <vrkit/Status.h>
#include <vrkit/Version.h>
#include <vrkit/scenedata/StatusPanelData.h>
#include <vrkit/plugin/Creator.h>
#include <vrkit/plugin/Registry.h>
#include <vrkit/plugin/TypedInitRegistryEntry.h>
#include <vrkit/plugin/Info.h>
#include <vrkit/plugin/Helpers.h>
#include <vrkit/exceptions/PluginException.h>

#include "ModeSwitchPlugin.h"


using namespace boost::assign;

static const vrkit::plugin::Info sInfo(
   "com.infiscape", "ModeSwitchPlugin",
   list_of(VRKIT_VERSION_MAJOR)(VRKIT_VERSION_MINOR)(VRKIT_VERSION_PATCH)
);
static vrkit::plugin::Creator<vrkit::viewer::Plugin> sPluginCreator(
   boost::bind(&vrkit::ModeSwitchPlugin::create, sInfo)
);

extern "C"
{

/** @name Plug-in Entry Points */
//@{
VRKIT_PLUGIN_API(const vrkit::plugin::Info*) getPluginInfo()
{
   return &sInfo;
}

VRKIT_PLUGIN_API(void) getPluginInterfaceVersion(vpr::Uint32& majorVer,
                                                 vpr::Uint32& minorVer)
{
   majorVer = VRKIT_PLUGIN_API_MAJOR;
   minorVer = VRKIT_PLUGIN_API_MINOR;
}

VRKIT_PLUGIN_API(vrkit::plugin::CreatorBase*) getCreator()
{
   return &sPluginCreator;
}
//@}

}

namespace
{

const vpr::GUID mode_switch_status_id("8c0034da-b613-4bd0-9c30-f8c35f48ba1a");

}

namespace vrkit
{

std::string ModeSwitchPlugin::getDescription()
{
   if ( mPlugins.empty() )
   {
      return std::string("Mode: NONE");
   }
   else
   {
      return std::string("Mode: ") + mModeNames[mCurrentMode];
   }
}

viewer::PluginPtr ModeSwitchPlugin::init(ViewerPtr viewer)
{
   const std::string plugin_path_prop("plugin_path");
   const std::string plugins_prop("plugins");
   const std::string mode_names_prop("mode_names");
   const std::string swap_button_prop("swap_button_nums");
   const std::string mode_plugin_type("mode_plugin_def");
   const std::string active_modes_prop("active_modes");
   const std::string plugin_prop("plugin");

   const unsigned int req_cfg_version(2);

   InterfaceTrader& if_trader = viewer->getUser()->getInterfaceTrader();
   mWandInterface = if_trader.getWandInterface();

   // -- Configure -- //
   std::string elt_type_name = getElementType();
   jccl::ConfigElementPtr elt =
      viewer->getConfiguration().getConfigElement(elt_type_name);

   if ( ! elt )
   {
      throw PluginException("ModeSwitchPlugin not find its configuration.",
                            VRKIT_LOCATION);
   }

   vprASSERT(elt->getID() == getElementType());

   // Check for correct version of plug-in configuration.
   if ( elt->getVersion() < req_cfg_version )
   {
      std::ostringstream msg;
      msg << "ModeSwitchPlugin: Configuration failed. Required cfg version: "
          << req_cfg_version << " found:" << elt->getVersion();
      throw PluginException(msg.str(), VRKIT_LOCATION);
   }

   // ---- Process configuration --- //
   mMaxMode = 0;

   // -- Setup the plug-in search path -- //
   std::vector<std::string> search_path;
   const unsigned int num_paths(elt->getNum(plugin_path_prop));

   for ( unsigned int i = 0; i < num_paths; ++i )
   {
      search_path.push_back(
         elt->getProperty<std::string>(plugin_path_prop, i)
      );
   }

   const std::vector<vpr::LibraryPtr> modules =
      plugin::findModules(search_path);
   std::for_each(
      modules.begin(), modules.end(),
      boost::bind(&ModeSwitchPlugin::registerModule, this, _1, viewer)
   );

   // Get the button for swapping
   mSwitchButton.configure(elt->getProperty<std::string>(swap_button_prop),
                           mWandInterface);

   // Get mode names
   const unsigned int num_mode_names(elt->getNum(mode_names_prop));
   mMaxMode = num_mode_names - 1;
   for ( unsigned int i = 0; i < num_mode_names; ++i )
   {
      mModeNames.push_back(elt->getProperty<std::string>(mode_names_prop, i));
   }

   // --- Load the managed plug-ins --- //
   const unsigned int num_plugins(elt->getNum(plugins_prop));

   VRKIT_STATUS << "[Mode Switch Plug-in] Found " << num_plugins
                << " plug-ins to load." << std::endl;

   plugin::RegistryPtr plugin_registry = viewer->getPluginRegistry();

   // Attempt to load each plug-in
   // - Get creator and create plug-in
   // - Push onto plug-in list
   for ( unsigned int i = 0; i < num_plugins; ++i )
   {
      PluginData plugin_data;
      jccl::ConfigElementPtr plg_elt =
         elt->getProperty<jccl::ConfigElementPtr>(plugins_prop, i);
      plugin_data.mName = plg_elt->getProperty<std::string>(plugin_prop);

      try
      {
         std::cout << "   Loading plug-in: " << plugin_data.mName << " ... "
                   << std::flush;

         std::vector<AbstractPluginPtr> deps;
         AbstractPluginPtr p =
            plugin_registry->makeInstance(plugin_data.mName, deps);

         std::vector<PluginData> plugins;
         plugins.reserve(deps.size() + 1);

         typedef std::vector<AbstractPluginPtr>::iterator iter_type;
         for ( iter_type d = deps.begin(); d != deps.end(); ++d )
         {
            viewer::PluginPtr cur_dep =
               boost::dynamic_pointer_cast<viewer::Plugin>(*d);

            if ( cur_dep )
            {
               PluginData dep_data;
               dep_data.mName   = cur_dep->getInfo().getName();
               dep_data.mPlugin = cur_dep;
               plugins.push_back(dep_data);
            }
         }

         plugin_data.mPlugin = boost::dynamic_pointer_cast<Plugin>(p);

         if ( ! plugin_data.mPlugin )
         {
            std::ostringstream msg_stream;
            msg_stream << "Invalid plug-in type '"
                       << p->getInfo().getFullName()
                       << "' given as plug-in for the Mode Switch Plug-in!";
            throw PluginException(msg_stream.str(), VRKIT_LOCATION);
         }

         plugins.push_back(plugin_data);

         const unsigned int num_active_modes =
            plg_elt->getNum(active_modes_prop);
         for ( unsigned int m = 0; m < num_active_modes; ++m )
         {
            const unsigned int mode_num =
               plg_elt->getProperty<unsigned int>(active_modes_prop, m);

            if ( mode_num > mMaxMode )
            {
               mMaxMode = mode_num;
            }

            typedef std::vector<PluginData>::iterator piter_type;
            for ( piter_type p = plugins.begin(); p != plugins.end(); ++p )
            {
               (*p).mActiveModes.push_back(mode_num);
            }
         }

         mPlugins.insert(mPlugins.end(), plugins.begin(), plugins.end());
         std::cout << "[OK]" << std::endl;
      }
      catch (std::runtime_error& ex)
      {
         std::cerr << "[FAILED]\n"
                   << "WARNING: ModeSwitchPlugin failed to load plug-in '"
                   << plugin_data.mName << "': " << ex.what() << std::endl;
      }
   }

   // Make sure we have enough mode names to match up with the number of modes.
   if ( mMaxMode >= mModeNames.size() )
   {
      mModeNames.resize(mMaxMode + 1, std::string("Unknown Mode"));
   }

   switchToMode(0, viewer);

   return shared_from_this();
}

void ModeSwitchPlugin::contextInit(ViewerPtr viewer)
{
   // Inform all plug-ins of context initialization.
   typedef std::vector<PluginData>::iterator iter_type;
   for ( iter_type i = mPlugins.begin(); i != mPlugins.end(); ++i )
   {
      (*i).mPlugin->contextInit(viewer);
   }
}

void ModeSwitchPlugin::update(ViewerPtr viewer)
{
   if ( isFocused() )
   {
      if ( mSwitchButton() )
      {
         unsigned int new_mode(0);
         if ( mMaxMode != 0 )
         {
            new_mode = (mCurrentMode + 1) % (mMaxMode + 1);
         }
         switchToMode(new_mode, viewer);
      }
   }

   typedef std::vector<PluginData>::iterator iter_type;
   for ( iter_type i = mPlugins.begin(); i != mPlugins.end(); ++i )
   {
      (*i).mPlugin->update(viewer);
   }
}

void ModeSwitchPlugin::contextPreDraw(ViewerPtr viewer)
{
   // Only tell the focused plug-in(s) to perform context-specific pre-draw
   // operations.
   typedef std::vector<PluginData>::iterator iter_type;
   for ( iter_type i = mPlugins.begin(); i != mPlugins.end(); ++i )
   {
      if ( (*i).mPlugin->isFocused() )
      {
         (*i).mPlugin->contextPreDraw(viewer);
      }
   }
}

void ModeSwitchPlugin::draw(ViewerPtr viewer)
{
   // Only tell the focused plug-in(s) to perform context-specific draw
   // operations.
   typedef std::vector<PluginData>::iterator iter_type;
   for ( iter_type i = mPlugins.begin(); i != mPlugins.end(); ++i )
   {
      if ( (*i).mPlugin->isFocused() )
      {
         (*i).mPlugin->draw(viewer);
      }
   }
}

void ModeSwitchPlugin::contextPostDraw(ViewerPtr viewer)
{
   // Only tell the focused plug-in(s) to perform context-specific post-draw
   // operations.
   typedef std::vector<PluginData>::iterator iter_type;
   for ( iter_type i = mPlugins.begin(); i != mPlugins.end(); ++i )
   {
      if ( (*i).mPlugin->isFocused() )
      {
         (*i).mPlugin->contextPostDraw(viewer);
      }
   }
}

void ModeSwitchPlugin::contextClose(ViewerPtr viewer)
{
   // Inform all plug-ins of context shutdown.
   typedef std::vector<PluginData>::iterator iter_type;
   for ( iter_type i = mPlugins.begin(); i != mPlugins.end(); ++i )
   {
      (*i).mPlugin->contextClose(viewer);
   }
}

void ModeSwitchPlugin::registerModule(vpr::LibraryPtr module,
                                      ViewerPtr viewer)
{
   viewer->getPluginRegistry()->addEntry(
      plugin::TypedInitRegistryEntry<viewer::Plugin>::create(
         module, &viewer::Plugin::validatePluginLib,
         boost::bind(&viewer::Plugin::init, _1, viewer)
      )
   );
}

void ModeSwitchPlugin::switchToMode(const unsigned int modeNum,
                                    ViewerPtr viewer)
{
   //mode_switch_status_id
   if ( modeNum > mMaxMode )
   {
      std::cerr << "ModeSwitchPlugin: Attempted to switch out of range to: "
                << modeNum << ".  Ignoring." << std::endl;
      return;
   }

   VRKIT_STATUS << "Switching to mode: " << mModeNames[modeNum] << std::endl;

   StatusPanelDataPtr status_panel_data =
      viewer->getSceneObj()->getSceneData<StatusPanelData>();

   status_panel_data->setHeaderTitle("Mode");
   status_panel_data->setCenterTitle("Controls");
   std::ostringstream stream;
   stream << mModeNames[modeNum];
   status_panel_data->setHeaderText(stream.str());

   status_panel_data->setControlText(mSwitchButton.toString(),
                                     "Switch Mode");

   for ( unsigned int i = 0; i < mPlugins.size(); ++i )
   {
      PluginData plugin = mPlugins[i];

      // If found mode in active modes
      if(std::find(plugin.mActiveModes.begin(), plugin.mActiveModes.end(), modeNum)
            != plugin.mActiveModes.end())
      {
         plugin.mPlugin->setFocused(viewer, true);
      }
      else
      {
         plugin.mPlugin->setFocused(viewer, false);
      }
   }

   mCurrentMode = modeNum;
}

}
