#include <vpr/vpr.h>
#include <vpr/DynLoad/LibraryLoader.h>
#include <jccl/Config/ConfigElement.h>

#include <OpenSG/VRJ/Viewer/IOV/InterfaceTrader.h>
#include <OpenSG/VRJ/Viewer/IOV/User.h>
#include <OpenSG/VRJ/Viewer/IOV/Viewer.h>
#include <OpenSG/VRJ/Viewer/IOV/WandInterface.h>
#include <OpenSG/VRJ/Viewer/IOV/PluginCreator.h>
#include <OpenSG/VRJ/Viewer/IOV/PluginHandler.h>
#include <OpenSG/VRJ/Viewer/plugins/ModeSwitchPlugin.h>


static inf::PluginCreator sPluginCreator("Mode Switch Plug-in");

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

IOV_PLUGIN_API(inf::PluginCreator*) create()
{
   sPluginCreator.setPlugin(inf::ModeSwitchPlugin::create());
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

   // Store this for initializing plug-ins when config() is invoked.
   mViewer = viewer;
}

bool ModeSwitchPlugin::canHandleElement(jccl::ConfigElementPtr elt)
{
   bool can_handle(elt->getID() == getElementType());

   if ( ! can_handle && ! mPlugins.empty() )
   {
      std::vector<PluginPtr>::iterator i;
      for ( i = mPlugins.begin(); i != mPlugins.end(); ++i )
      {
         if ( (*i)->canHandleElement(elt) )
         {
            can_handle = true;
            break;
         }
      }
   }

   return can_handle;
}

bool ModeSwitchPlugin::config(jccl::ConfigElementPtr elt)
{
   bool status(false);

   if ( elt->getID() == getElementType() )
   {
      const std::string plugin_path_prop("plugin_path");
      const std::string plugin_prop("plugin");

      std::vector<std::string> search_path;
      search_path.push_back("plugins");

      const unsigned int num_paths(elt->getNum(plugin_path_prop));

      for ( unsigned int i = 0; i < num_paths; ++i )
      {
         search_path.push_back(
            elt->getProperty<std::string>(plugin_path_prop, i)
         );
      }

      const unsigned int num_plugins(elt->getNum(plugin_prop));

      for ( unsigned int i = 0; i < num_plugins; ++i )
      {
         std::string plugin_name = elt->getProperty<std::string>(plugin_prop,
                                                                 i);
         vpr::LibraryPtr dso = vpr::LibraryLoader::findDSO(plugin_name,
                                                           search_path);

         if ( dso.get() != NULL )
         {
            VersionCheckCallable version_functor;

            vpr::ReturnStatus version_status =
               vpr::LibraryLoader::findEntryPoint(dso,
                                                  PluginHandler::GET_VERSION_FUNC,
                                                  version_functor);

            if ( ! version_status.success() )
            {
               std::cerr << "Version mismatch!  Plug-in '" << plugin_name
                         << "' cannot be used." << std::endl;
            }
            else
            {
               inf::ModeSwitchPluginPtr handler = shared_from_this();
               PluginCreateCallable<ModeSwitchPluginPtr> create_functor(handler);

               vpr::ReturnStatus create_status =
                  vpr::LibraryLoader::findEntryPoint(dso,
                                                     PluginHandler::CREATE_FUNC,
                                                     create_functor);

               if ( create_status.success() )
               {
                  mLoadedDsos.push_back(dso);
               }
            }
         }
         else
         {
            std::cerr << "WARNING: Failed to load plug-in '" << plugin_name
                      << "'!" << std::endl;
         }
      }

      status = true;
   }
   else if ( ! mPlugins.empty() )
   {
      std::vector<PluginPtr>::iterator i;
      for ( i = mPlugins.begin(); i != mPlugins.end(); ++i )
      {
         if ( (*i)->canHandleElement(elt) )
         {
            status = (*i)->config(elt);
            break;
         }
      }
   }

   return status;
}

void ModeSwitchPlugin::update(inf::ViewerPtr viewer)
{
   gadget::DigitalInterface& switch_button =
      mWandInterface->getButton(SWITCH_BUTTON);

   if ( switch_button->getData() == gadget::Digital::TOGGLE_ON )
   {
      mCurrentMode = (mCurrentMode + 1) % mPlugins.size();
   }

   if ( ! mPlugins.empty() )
   {
      mPlugins[mCurrentMode]->update(viewer);
   }
}

void ModeSwitchPlugin::addPlugin(inf::PluginPtr plugin)
{
   plugin->init(mViewer);
   mPlugins.push_back(plugin);
}

}
