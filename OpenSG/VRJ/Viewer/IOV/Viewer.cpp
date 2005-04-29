#include <algorithm>

#include <vpr/vpr.h>
#include <vpr/DynLoad/LibraryLoader.h>
#include <jccl/Config/Configuration.h>

#include <OpenSG/VRJ/Viewer/IOV/User.h>
#include <OpenSG/VRJ/Viewer/IOV/Plugin.h>
#include <OpenSG/VRJ/Viewer/IOV/PluginHandler.h>
#include <OpenSG/VRJ/Viewer/IOV/Viewer.h>


namespace
{

struct ElementRemovePredicate
{
   ElementRemovePredicate(inf::PluginPtr plugin)
      : mPlugin(plugin)
   {
   }

   bool operator()(jccl::ConfigElementPtr e)
   {
      if ( mPlugin->canHandleElement(e) && mPlugin->config(e) )
      {
         return true;
      }
      else
      {
         return false;
      }
   }

   inf::PluginPtr mPlugin;
};

}

namespace inf
{

void Viewer::init()
{
   vrj::OpenSGApp::init();

   bool cfg_loaded(false);
   jccl::Configuration cfg;

   if ( ! mCfgFile.empty() )
   {
      cfg_loaded = cfg.load(mCfgFile);
   }

   if ( ! cfg_loaded )
   {
      std::cerr << "WARNING: Failed to load our configuration!" << std::endl;
   }

   // Create an initialize the user
   mUser = User::create();
   mUser->init();

   // Create and initialize the base scene object
   mScene = Scene::create();
   mScene->init();

   if ( cfg_loaded )
   {
      const std::string app_elt_type("infiscape_opensg_viewer");
      const std::string plugin_path_prop("plugin_path");
      const std::string plugin_prop("plugin");

      std::vector<jccl::ConfigElementPtr> app_elts;

      cfg.getByType(app_elt_type, app_elts);

      if ( ! app_elts.empty() )
      {
         jccl::ConfigElementPtr app_cfg = app_elts[0];

         std::vector<std::string> search_path;
         search_path.push_back("plugins");

         const unsigned int num_paths(app_cfg->getNum(plugin_path_prop));

         for ( unsigned int i = 0; i < num_paths; ++i )
         {
            search_path.push_back(
               app_cfg->getProperty<std::string>(plugin_path_prop, i)
            );
         }

         std::vector<jccl::ConfigElementPtr> all_elts = cfg.vec();

         // Remove app_cfg from all_elts since we are the consumer for that
         // element.
         all_elts.erase(std::remove(all_elts.begin(), all_elts.end(), app_cfg),
                        all_elts.end());

         const unsigned int num_plugins(app_cfg->getNum(plugin_prop));

         for ( unsigned int i = 0; i < num_plugins; ++i )
         {
            std::string plugin_name =
               app_cfg->getProperty<std::string>(plugin_prop, i);
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
                  inf::ViewerPtr viewer = shared_from_this();
                  PluginCreateCallable<ViewerPtr> create_functor(viewer);

                  vpr::ReturnStatus create_status =
                     vpr::LibraryLoader::findEntryPoint(dso,
                                                        PluginHandler::CREATE_FUNC,
                                                        create_functor);

                  if ( create_status.success() )
                  {
                     // At this point, the plug-in has been instantiated,
                     // added to our collection of loaded plug-ins, and
                     // initialized.

                     mLoadedDsos.push_back(dso);

                     // The newly created plug-in will be at the end of
                     // mPlugins.
                     // XXX: This is a bit dodgy...
                     inf::PluginPtr plugin = mPlugins[mPlugins.size() -1 ];

                     // Configure the newly loaded plug-in and remove all the
                     // elements (if any) from all_elts that the plug-in
                     // consumes.
                     ElementRemovePredicate remove_pred(plugin);
                     std::vector<jccl::ConfigElementPtr>::iterator new_end =
                        std::remove_if(all_elts.begin(), all_elts.end(),
                                       remove_pred);
                     all_elts.erase(new_end, all_elts.end());
                  }
               }
            }
            else
            {
               std::cerr << "WARNING: Failed to load plug-in '" << plugin_name
                         << "'!" << std::endl;
            }
         }

         if ( ! all_elts.empty() )
         {
            std::cout << "Unconsumed config elements from "
                      << cfg.getFileName() << ":\n";

            std::vector<jccl::ConfigElementPtr>::iterator i;
            for ( i = all_elts.begin(); i != all_elts.end(); ++i )
            {
               std::cout << "\t" << (*i)->getName() << "\n";
            }

            std::cout << std::flush;
         }
      }
   }
}

void Viewer::preFrame()
{
   std::vector<inf::PluginPtr>::iterator i;
   for ( i = mPlugins.begin(); i != mPlugins.end(); ++i )
   {
      (*i)->update(shared_from_this());
   }

   // Update the user (and navigation)
   getUser()->update(shared_from_this());
}

void Viewer::addPlugin(PluginPtr plugin)
{
   plugin->init(shared_from_this());
   mPlugins.push_back(plugin);
}

}
