#include <algorithm>

#include <vpr/vpr.h>
#include <vpr/DynLoad/LibraryLoader.h>
#include <jccl/Config/Configuration.h>

#include <OpenSG/VRJ/Viewer/User.h>
#include <OpenSG/VRJ/Viewer/Plugin.h>
#include <OpenSG/VRJ/Viewer/Viewer.h>


namespace
{

struct VersionCheckCallable
{
   VersionCheckCallable()
   {
   }

   bool operator()(void* func)
   {
      void (*version_func)(vpr::Uint32&, vpr::Uint32&);
      version_func = (void (*)(vpr::Uint32&, vpr::Uint32&)) func;

      unsigned int major_ver;
      unsigned int minor_ver;
      (*version_func)(major_ver, minor_ver);

      // TODO: Now do something with the version info that we got back ...

      return true;
   }

   static const vpr::Uint32 sPluginMajor;
   static const vpr::Uint32 sPluginMinor;
};

const vpr::Uint32 VersionCheckCallable::sPluginMajor(INF_PLUGIN_API_MAJOR);
const vpr::Uint32 VersionCheckCallable::sPluginMinor(INF_PLUGIN_API_MINOR);

struct PluginCreateCallable
{
   PluginCreateCallable(inf::ViewerPtr viewer)
      : mViewer(viewer)
   {
   }

   bool operator()(void* func)
   {
      inf::PluginPtr (*create_func)();

      create_func = (inf::PluginPtr (*)()) func;
      inf::PluginPtr plugin = (*create_func)();

      if ( NULL == plugin.get() )
      {
         std::cerr << "Plug-in creation failed." << std::endl;
      }
      else
      {
         std::cout << "Initializing the plug-in ..." << std::endl;
         // Initialize the newly loaded plug-in.
         plugin->init(mViewer);

         mViewer->addPlugin(plugin);
      }

      return true;
   }

   inf::ViewerPtr mViewer;
};

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

   jccl::Configuration cfg;
   bool cfg_loaded = cfg.load("viewer.jconf");

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

         const std::string get_version_func("getPluginInterfaceVersion");
         const std::string create_func("create");

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
                  vpr::LibraryLoader::findEntryPoint(dso, get_version_func,
                                                     version_functor);

               if ( ! version_status.success() )
               {
                  std::cerr << "Version mismatch!  Plug-in '" << plugin_name
                            << "' cannot be used." << std::endl;
               }
               else
               {
                  PluginCreateCallable create_functor(shared_from_this());

                  vpr::ReturnStatus create_status =
                     vpr::LibraryLoader::findEntryPoint(dso, create_func,
                                                        create_functor);

                  if ( create_status.success() )
                  {
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

}
