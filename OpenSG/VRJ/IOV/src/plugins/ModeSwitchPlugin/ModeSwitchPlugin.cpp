#include <OpenSG/OSGConfig.h>

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

   // Lookup details from config element
   const std::string plugin_path_prop("plugin_path");
   const std::string plugin_prop("plugin");

   // Setup search path
   std::vector<std::string> search_path;

   // Set up two default search paths:
   //    1. Relative path to './plugins'
   //    2. IOV_BASE_DIR/lib/IOV/plugins
   search_path.push_back("plugins");

   std::string iov_base_dir;
   if ( vpr::System::getenv("IOV_BASE_DIR", iov_base_dir).success() )
   {
      fs::path iov_base_path(iov_base_dir, fs::native);
      fs::path def_iov_plugin_path = iov_base_path / "lib/IOV/plugins";

      if ( fs::exists(def_iov_plugin_path) )
      {
         std::string def_search_path =
            def_iov_plugin_path.native_directory_string();
         std::cout << "Setting default IOV plug-in path: " << def_search_path
                   << std::endl;
         search_path.push_back(def_search_path);
      }
      else
      {
         std::cerr << "Default IOV plug-in path does not exist: "
                   << def_iov_plugin_path.native_directory_string()
                   << std::endl;
      }
   }

   const unsigned int num_paths(elt->getNum(plugin_path_prop));

   for ( unsigned int i = 0; i < num_paths; ++i )
   {
      search_path.push_back(
         elt->getProperty<std::string>(plugin_path_prop, i)
      );
   }

   mPluginFactory = inf::PluginFactory::create();
   mPluginFactory->init(search_path);

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
            mPluginFactory->getPluginCreator(plugin_name);

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
      mWandInterface->getButton(SWITCH_BUTTON);

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
