#include <algorithm>

#include <OpenSG/OSGConnectionFactory.h>
#include <OpenSG/OSGSimpleAttachments.h>

#include <vpr/vpr.h>
#include <vpr/DynLoad/LibraryLoader.h>
#include <vpr/IO/Socket/InetAddr.h>
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

Viewer::~Viewer()
{
   if ( NULL != mAspect )
   {
      delete mAspect;
   }

   if ( NULL != mConnection )
   {
      delete mConnection;
   }
}

void Viewer::init()
{
   // This has to be called before OSG::osgInit(), which is done by
   // vrj::OpenSGApp::init().
   OSG::ChangeList::setReadWriteDefault();

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
      const std::string root_name_prop("root_name");
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

         // Set ourselves up as a rendering master (or not depending on the
         // configuration).
         configureNetwork(app_cfg);

         std::string root_name =
            app_cfg->getProperty<std::string>(root_name_prop);

         // This has to be done after the slave connections are received so
         // that this change is included with the initial sync.
         CoredGroupPtr root_node = mScene->getSceneRoot();
         OSG::beginEditCP(root_node);
            OSG::setName(root_node.node(), root_name);
         OSG::endEditCP(root_node);

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
               const std::string get_ver_func(PluginHandler::GET_VERSION_FUNC);
               VersionCheckCallable version_functor;

               vpr::ReturnStatus version_status =
                  vpr::LibraryLoader::findEntryPoint(dso, get_ver_func,
                                                     version_functor);

               if ( ! version_status.success() )
               {
                  std::cerr << "Version mismatch!  Plug-in '" << plugin_name
                            << "' cannot be used." << std::endl;
               }
               else
               {
                  const std::string get_creator_func(PluginHandler::GET_CREATOR_FUNC);
                  inf::ViewerPtr viewer = shared_from_this();
                  PluginCreateCallable<ViewerPtr> create_functor(viewer);

                  vpr::ReturnStatus create_status =
                     vpr::LibraryLoader::findEntryPoint(dso, get_creator_func,
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

   // First, we update the state of each plug-in.  All plug-ins will get a
   // consistent view of the run-time state of the system before being run.
   for ( i = mPlugins.begin(); i != mPlugins.end(); ++i )
   {
      (*i)->updateState(shared_from_this());
   }

   // Then, we tell each plug-in to do its thing.  Any given plug-in may
   // change the state of the system as a result of performing its task(s).
   for ( i = mPlugins.begin(); i != mPlugins.end(); ++i )
   {
      (*i)->run(shared_from_this());
   }

   // Update the user (and navigation)
   getUser()->update(shared_from_this());

   if ( NULL != mConnection )
   {
      try
      {
         int finish(0);

         mConnection->signal();
         mAspect->sendSync(*mConnection, OSG::Thread::getCurrentChangeList());
         mConnection->putValue(finish);
         mConnection->flush();

         OSG::Thread::getCurrentChangeList()->clearAll();
      }
      catch (OSG::Exception& ex)
      {
         std::cerr << ex.what() << std::endl;
         // XXX: How do we find out which channel caused the exception to be
         // thrown so that we can disconnect from it?
//         mConnection->disconnect();
         // XXX: We should not be dropping the connection with all nodes just
         // because one (or more) may have gone away.
         delete mConnection;
         mConnection = NULL;
      }
   }
}

void Viewer::addPlugin(PluginPtr plugin)
{
   plugin->setFocused(true);
   plugin->init(shared_from_this());
   mPlugins.push_back(plugin);
}

void Viewer::configureNetwork(jccl::ConfigElementPtr appCfg)
{
   const std::string listen_port_prop("listen_port");
   const std::string slave_count_prop("slave_count");

   const unsigned short listen_port =
      appCfg->getProperty<unsigned short>(listen_port_prop);
   const unsigned int slave_count =
      appCfg->getProperty<unsigned int>(slave_count_prop);

   if ( listen_port != 0 && slave_count != 0 )
   {
      vpr::InetAddr local_host_addr;
      if ( vpr::InetAddr::getLocalHost(local_host_addr).success() )
      {
         mAspect = new OSG::RemoteAspect();
         mConnection =
            OSG::ConnectionFactory::the().createGroup("StreamSock");
         local_host_addr.setPort(listen_port);
         std::stringstream addr_stream;
         addr_stream << local_host_addr.getAddressString() << ":"
                     << listen_port;
         mConnection->bind(addr_stream.str());
      }

      mChannels.resize(slave_count);

      for ( unsigned int s = 0; s < slave_count; ++s )
      {
         std::cout << "Waiting for slave #" << s << " to connect ..."
                   << std::endl;
         mChannels[s] = mConnection->acceptPoint();
      }

      // NOTE: We are not clearing the change list at this point
      // because that would blow away any actions taken during the
      // initialization of mScene.

      std::cout << "All " << slave_count << " nodes have connected"
                << std::endl;
   }
}

}
