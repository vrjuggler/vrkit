// Copyright (C) Infiscape Corporation 2005

#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#endif

#include <iostream>
#include <sstream>

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

#include "ModeSwitchPlugin.h"


namespace fs = boost::filesystem;

static inf::PluginCreator sPluginCreator(&inf::ModeSwitchPlugin::create,
                                         "Mode Switch Plug-in");

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

IOV_PLUGIN_API(inf::PluginCreator*) getCreator()
{
   return &sPluginCreator;
}
//@}

}

namespace
{
   const vpr::GUID mode_switch_status_id("8c0034da-b613-4bd0-9c30-f8c35f48ba1a");
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

void ModeSwitchPlugin::init(inf::ViewerPtr viewer)
{
   const std::string plugin_path_prop("plugin_path");
   const std::string plugins_prop("plugins");
   const std::string mode_names_prop("mode_names");
   const std::string swap_button_prop("swap_button_num");
   const std::string mode_plugin_type("mode_plugin_def");
   const std::string active_modes_prop("active_modes");
   const std::string plugin_prop("plugin");

   const unsigned int req_cfg_version(1);

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
   mSwitchButton = elt->getProperty<int>(swap_button_prop);

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
         inf::PluginCreator* creator =
            plugin_factory->getPluginCreator(plugin_data.mName);

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
         }
         else
         {
            std::cerr << "[ModeSwitchPlugin] ERROR: Plug-in '"
                      << plugin_data.mName << "' has a NULL creator!"
                      << std::endl;
         }
      }
      catch (std::runtime_error& ex)
      {
         std::cerr << "WARNING: ModeSwitchPlugin failed to load plug-in '"
                   << plugin_data.mName << "': " << ex.what() << std::endl;
      }
   }

   // Make sure we have enough mode names to match up with the number of modes.
   if(mMaxMode >= mModeNames.size())
   {
      mModeNames.resize(mMaxMode+1, std::string("Unknown Mode"));
   }

   switchToMode(0, viewer);
}

void ModeSwitchPlugin::updateState(inf::ViewerPtr viewer)
{
   gadget::DigitalInterface& switch_button =
      mWandInterface->getButton(mSwitchButton);

   if ( switch_button->getData() == gadget::Digital::TOGGLE_ON )
   {
      unsigned int new_mode(0);
      if ( mMaxMode != 0 )
      {
         new_mode = (mCurrentMode + 1) % (mMaxMode + 1);
      }
      switchToMode(new_mode, viewer);
   }

   for ( unsigned int i = 0; i < mPlugins.size(); ++i )
   {
      if ( mPlugins[i].mPlugin->isFocused() )
      {
         mPlugins[i].mPlugin->updateState(viewer);
      }
   }
}

void ModeSwitchPlugin::run(inf::ViewerPtr viewer)
{
   std::vector<PluginData>::iterator i;
   for ( i = mPlugins.begin(); i != mPlugins.end(); ++i )
   {
      (*i).mPlugin->run(viewer);
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

   StatusPanelPluginDataPtr status_panel_data =
      viewer->getSceneObj()->getSceneData<StatusPanelPluginData>(StatusPanelPluginData::type_guid);
   if(status_panel_data->mStatusPanelPlugin)
   {
      inf::StatusPanel& panel = status_panel_data->mStatusPanelPlugin->getPanel();
      panel.setHeaderTitle("Mode");
      panel.setCenterTitle("Controls");
      std::ostringstream stream;
      stream << mModeNames[modeNum];
      panel.setHeaderText(stream.str());
      panel.setControlText((StatusPanel::ControlTextLine) mSwitchButton,
                           "Switch Mode");
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
