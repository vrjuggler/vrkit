// Copyright (C) Infiscape Corporation 2005-2006

#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#endif

#include <iostream>
#include <sstream>
#include <algorithm>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <vpr/vpr.h>
#include <vpr/System.h>
#include <jccl/Config/ConfigElement.h>

#include <IOV/InterfaceTrader.h>
#include <IOV/User.h>
#include <IOV/Viewer.h>
#include <IOV/WandInterface.h>
#include <IOV/PluginCreator.h>
#include <IOV/PluginFactory.h>
#include <IOV/Util/Exceptions.h>
#include <IOV/Status.h>
#include <IOV/StatusPanel.h>
#include <IOV/StatusPanelPlugin.h>
#include <IOV/StatusPanelData.h>

#include "ModeSwitchPlugin.h"


namespace fs = boost::filesystem;

static inf::PluginCreator<inf::Plugin> sPluginCreator(
   &inf::ModeSwitchPlugin::create, "Mode Switch Plug-in"
);

extern "C"
{

/** @name Plug-in Entry Points */
//@{
IOV_PLUGIN_API(void) getPluginInterfaceVersion(vpr::Uint32& majorVer,
                                               vpr::Uint32& minorVer)
{
   majorVer = INF_PLUGIN_API_MAJOR;
   minorVer = INF_PLUGIN_API_MINOR;
}

IOV_PLUGIN_API(inf::PluginCreatorBase*) getCreator()
{
   return &sPluginCreator;
}
//@}

}

namespace
{

const vpr::GUID mode_switch_status_id("8c0034da-b613-4bd0-9c30-f8c35f48ba1a");

struct IncValue
{
   int operator()(int v)
   {
      return v + 1;
   }
};

}

namespace inf
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

PluginPtr ModeSwitchPlugin::init(inf::ViewerPtr viewer)
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
   jccl::ConfigElementPtr elt = viewer->getConfiguration().getConfigElement(elt_type_name);

   if(!elt)
   {
      throw PluginException("ModeSwitchPlugin not find its configuration.",
                            IOV_LOCATION);
   }

   vprASSERT(elt->getID() == getElementType());

   // Check for correct version of plugin configuration
   if(elt->getVersion() < req_cfg_version)
   {
      std::stringstream msg;
      msg << "ModeSwitchPlugin: Configuration failed. Required cfg version: " << req_cfg_version
          << " found:" << elt->getVersion();
      throw PluginException(msg.str(), IOV_LOCATION);
   }

   // ---- Process configuration --- //
   mMaxMode = 0;

   // -- Setup the plugin search path -- //
   std::vector<std::string> search_path;
   const unsigned int num_paths(elt->getNum(plugin_path_prop));

   for ( unsigned int i = 0; i < num_paths; ++i )
   {
      std::string new_path = elt->getProperty<std::string>(plugin_path_prop, i);
      search_path.push_back(new_path);
   }

   inf::PluginFactoryPtr plugin_factory = viewer->getPluginFactory();
   if(!search_path.empty())
   {
      plugin_factory->addScanPath(search_path);
   }

   // Get the button for swapping
   mSwitchButton.configButtons(elt->getProperty<std::string>(swap_button_prop));

   // Get mode names
   const unsigned int num_mode_names(elt->getNum(mode_names_prop));
   mMaxMode = num_mode_names - 1;
   for ( unsigned int i = 0; i < num_mode_names; ++i )
   {
      std::string mode_name = elt->getProperty<std::string>(mode_names_prop, i);
      mModeNames.push_back(mode_name);
   }

   // --- Load the managed plugins --- //
   const unsigned int num_plugins(elt->getNum(plugins_prop));

   std::cout << "ModeSwitchPlugin: Found " << num_plugins << " plugins to load." << std::endl;

   // Attempt to load each plugin
   // - Get creator and create plugin
   // - Push onto plugin list
   for ( unsigned int i = 0; i < num_plugins; ++i )
   {
      PluginData plugin_data;
      jccl::ConfigElementPtr plg_elt =
         elt->getProperty<jccl::ConfigElementPtr>(plugins_prop, i);
      plugin_data.mName = plg_elt->getProperty<std::string>(plugin_prop);

      try
      {
         std::cout << "   Loading plugin: " << plugin_data.mName << " .... "
                   << std::flush;

         inf::PluginCreator<inf::Plugin>* creator =
            plugin_factory->getPluginCreator<inf::Plugin>(plugin_data.mName);

         if ( NULL != creator )
         {
            plugin_data.mPlugin = creator->createPlugin();
            plugin_data.mPlugin->init(viewer);

            const unsigned int num_active_modes =
               plg_elt->getNum(active_modes_prop);
            for ( unsigned int m = 0; m < num_active_modes; ++m )
            {
               unsigned int mode_num =
                  plg_elt->getProperty<unsigned int>(active_modes_prop,m);
               if ( mode_num > mMaxMode )
               {
                  mMaxMode = mode_num;
               }

               plugin_data.mActiveModes.push_back(mode_num);
            }

            mPlugins.push_back(plugin_data);
            std::cout << "[OK]" << std::endl;
         }
         else
         {
            std::cerr << "[ERROR]\n   ModeSwitchPlugin ERROR: Plug-in '"
                      << plugin_data.mName << "' has a NULL creator!"
                      << std::endl;
         }
      }
      catch (std::runtime_error& ex)
      {
         std::cerr << "[FAILED]\n   WARNING: ModeSwitchPlugin failed to load plug-in '"
                   << plugin_data.mName << "': " << ex.what() << std::endl;
      }
   }

   // Make sure we have enough mode names to match up with the number of modes.
   if(mMaxMode >= mModeNames.size())
   {
      mModeNames.resize(mMaxMode+1, std::string("Unknown Mode"));
   }

   switchToMode(0, viewer);

   return shared_from_this();
}

void ModeSwitchPlugin::contextInit(inf::ViewerPtr viewer)
{
   // Inform all plug-ins of context initialization.
   std::vector<PluginData>::iterator i;
   for ( i = mPlugins.begin(); i != mPlugins.end(); ++i )
   {
      (*i).mPlugin->contextInit(viewer);
   }
}

void ModeSwitchPlugin::update(inf::ViewerPtr viewer)
{
   if ( isFocused() )
   {
      if ( mSwitchButton.test(mWandInterface, gadget::Digital::TOGGLE_ON) )
      {
         unsigned int new_mode(0);
         if ( mMaxMode != 0 )
         {
            new_mode = (mCurrentMode + 1) % (mMaxMode + 1);
         }
         switchToMode(new_mode, viewer);
      }
   }

   std::vector<PluginData>::iterator i;
   for ( i = mPlugins.begin(); i != mPlugins.end(); ++i )
   {
      (*i).mPlugin->update(viewer);
   }
}

void ModeSwitchPlugin::contextPreDraw(inf::ViewerPtr viewer)
{
   // Only tell the focused plug-in(s) to perform context-specific pre-draw
   // operations.
   std::vector<PluginData>::iterator i;
   for ( i = mPlugins.begin(); i != mPlugins.end(); ++i )
   {
      if ( (*i).mPlugin->isFocused() )
      {
         (*i).mPlugin->contextPreDraw(viewer);
      }
   }
}

void ModeSwitchPlugin::draw(inf::ViewerPtr viewer)
{
   // Only tell the focused plug-in(s) to perform context-specific draw
   // operations.
   std::vector<PluginData>::iterator i;
   for ( i = mPlugins.begin(); i != mPlugins.end(); ++i )
   {
      if ( (*i).mPlugin->isFocused() )
      {
         (*i).mPlugin->draw(viewer);
      }
   }
}

void ModeSwitchPlugin::contextPostDraw(inf::ViewerPtr viewer)
{
   // Only tell the focused plug-in(s) to perform context-specific post-draw
   // operations.
   std::vector<PluginData>::iterator i;
   for ( i = mPlugins.begin(); i != mPlugins.end(); ++i )
   {
      if ( (*i).mPlugin->isFocused() )
      {
         (*i).mPlugin->contextPostDraw(viewer);
      }
   }
}

void ModeSwitchPlugin::contextClose(inf::ViewerPtr viewer)
{
   // Inform all plug-ins of context shutdown.
   std::vector<PluginData>::iterator i;
   for ( i = mPlugins.begin(); i != mPlugins.end(); ++i )
   {
      (*i).mPlugin->contextClose(viewer);
   }
}

void ModeSwitchPlugin::switchToMode(const unsigned int modeNum,
                                    inf::ViewerPtr viewer)
{
   //mode_switch_status_id
   if(modeNum > mMaxMode)
   {
      std::cerr << "ModeSwitchPlugin: Attempted to switch out of range to: "
                << modeNum << ".  Ignoring." << std::endl;
      return;
   }

   IOV_STATUS << "Switching to mode: " << mModeNames[modeNum] << std::endl;

   StatusPanelDataPtr status_panel_data =
      viewer->getSceneObj()->getSceneData<StatusPanelData>();
   if(status_panel_data->mStatusPanelPlugin)
   {
      inf::StatusPanel& panel = status_panel_data->mStatusPanelPlugin->getPanel();
      panel.setHeaderTitle("Mode");
      panel.setCenterTitle("Controls");
      std::ostringstream stream;
      stream << mModeNames[modeNum];
      panel.setHeaderText(stream.str());

      // The button numbers in mSwitchButton are zero-based, but we would like
      // them to be one-based in the status panel display.
      std::vector<int> btns(mSwitchButton.getButtons().size());
      std::transform(mSwitchButton.getButtons().begin(),
                     mSwitchButton.getButtons().end(), btns.begin(),
                     IncValue());

      panel.setControlText(btns, "Switch Mode");
   }

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
