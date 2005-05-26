#ifdef WIN32
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
      return std::string("Mode: ") + mPlugins[mCurrentMode]->getDescription();
   }
}

void ModeSwitchPlugin::init(inf::ViewerPtr viewer)
{
   const std::string plugin_path_prop("plugin_path");
   const std::string plugin_prop("plugin");
   const std::string swap_button_prop("swap_button_num");
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

   // Get the button for swapping
   mSwitchButton = elt->getProperty<int>(swap_button_prop);

   // -- Setup the plugin search path -- //
   std::vector<std::string> search_path;
   const unsigned int num_paths(elt->getNum(plugin_path_prop));

   for ( unsigned int i = 0; i < num_paths; ++i )
   {
      search_path.push_back(
         elt->getProperty<std::string>(plugin_path_prop, i)
      );
   }

   inf::PluginFactoryPtr plugin_factory = viewer->getPluginFactory();
   if(!search_path.empty())
   {
      plugin_factory->addScanPath(search_path);
   }

   // --- Load the managed plugins --- //
   const unsigned int num_plugins(elt->getNum(plugin_prop));

   // Attempt to load each plugin
   // - Get creator and create plugin
   // - Push onto plugin list
   for ( unsigned int i = 0; i < num_plugins; ++i )
   {
      std::string plugin_name = elt->getProperty<std::string>(plugin_prop, i);
      try
      {
         inf::PluginCreator* creator =
            plugin_factory->getPluginCreator(plugin_name);

         if ( NULL != creator )
         {
            inf::PluginPtr plugin = creator->createPlugin();

            // If this is the first plug-in being added, then it will be
            // the one to get focus.
            if ( mPlugins.empty() )
            {
               plugin->setFocused(true);
            }
            // Otherwise, all other plug-ins remain unfocused until a
            // plug-in switch is performed.
            else
            {
               plugin->setFocused(false);
            }

            plugin->init(viewer);
            mPlugins.push_back(plugin);
         }
         else
         {
            std::cerr << "[ModeSwitchPlugin] ERROR: Plug-in '"
                      << plugin_name << "' has a NULL creator!"
                      << std::endl;
         }
      }
      catch (std::runtime_error& ex)
      {
         std::cerr << "WARNING: ModeSwitchPlugin failed to load plug-in '"
                   << plugin_name << "': " << ex.what() << std::endl;
      }
   }
}

void ModeSwitchPlugin::updateState(inf::ViewerPtr viewer)
{
   gadget::DigitalInterface& switch_button =
      mWandInterface->getButton(mSwitchButton);

   if ( switch_button->getData() == gadget::Digital::TOGGLE_ON )
   {
      const int old_mode(mCurrentMode);
      mCurrentMode = (mCurrentMode + 1) % mPlugins.size();

      if ( ! mPlugins.empty() )
      {
         mPlugins[old_mode]->setFocused(false);
         mPlugins[mCurrentMode]->setFocused(true);
         std::cout << getDescription() << std::endl;
      }
   }

   if ( ! mPlugins.empty() )
   {
      mPlugins[mCurrentMode]->updateState(viewer);
   }
}

void ModeSwitchPlugin::run(inf::ViewerPtr viewer)
{
   std::vector<PluginPtr>::iterator i;
   for ( i = mPlugins.begin(); i != mPlugins.end(); ++i )
   {
      (*i)->run(viewer);
   }
}

}
