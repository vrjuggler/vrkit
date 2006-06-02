// Copyright (C) Infiscape Corporation 2005-2006

#include <algorithm>
#include <boost/bind.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <OpenSG/OSGRemoteAspect.h>
#include <OpenSG/OSGGroupConnection.h>
#include <OpenSG/OSGConnectionFactory.h>
#include <OpenSG/OSGSimpleAttachments.h>

#include <vpr/vpr.h>
#include <vpr/System.h>
#include <vpr/Util/FileUtils.h>
#include <jccl/Config/Configuration.h>

#include <IOV/EventData.h>
#include <IOV/User.h>
#include <IOV/Grab/IntersectionStrategy.h>
#include <IOV/Plugin.h>
#include <IOV/PluginCreator.h>
#include <IOV/PluginFactory.h>
#include <IOV/Viewer.h>
#include <IOV/SceneObject.h>
#include <IOV/Status.h>
#include <IOV/Util/OpenSGHelpers.h>

// Embedded plugins
#include <IOV/StatusPanelPlugin.h>

namespace fs = boost::filesystem;

namespace inf
{

Viewer::Viewer()
   : vrj::OpenSGApp(NULL)
   , mAspect(NULL)
   , mConnection(NULL)
{
   mPluginFactory = PluginFactory::create();
}


Viewer::~Viewer()
{
}

void Viewer::init()
{
   // This has to be called before OSG::osgInit(), which is done by
   // vrj::OpenSGApp::init().
   OSG::ChangeList::setReadWriteDefault();

   vrj::OpenSGApp::init();

   // Verify configuration has been loaded
   bool have_config = (mConfiguration.getAllConfigElements().size() > 0);
   if (!have_config)
   {
      std::cerr << "WARNING: No configuration files were provided to the "
                << "Viewer application!" << std::endl;
   }

   // Create an initialize the user
   mUser = User::create()->init();

   // Create and initialize the base scene object
   mScene = Scene::create()->init();

   mEventData = mScene->getSceneData<EventData>();

   // Load plugins embedded in library
   getPluginFactory()->registerCreator(
      new inf::PluginCreator<inf::Plugin>(&inf::StatusPanelPlugin::create,
                                          "StatusPanelPlugin"),
      "StatusPanelPlugin"
   );

   // Load the app configuration and then...
   // - Setup scene root for networking
   // - Configure the networking
   // - Load and initialize the plugins
   if ( have_config )
   {
      const std::string app_elt_type("infiscape_opensg_viewer");
      const std::string cluster_elt_type("iov_cluster");
      const std::string root_name_prop("root_name");

      // -- Configure core application -- //
      jccl::ConfigElementPtr app_cfg =
         mConfiguration.getConfigElement(app_elt_type);

      if ( app_cfg )
      {
         const unsigned int app_cfg_ver(4);
         if ( app_cfg->getVersion() < app_cfg_ver )
         {
            std::cerr << "WARNING: IOV config element '" << app_cfg->getName()
                      << "' is out of date!" << std::endl
                      << "         Current config element version is "
                      << app_cfg_ver << ", but this one is version "
                      << app_cfg->getVersion() << std::endl
                      << "         IOV configuration may fail or be incomplete."
                      << std::endl;
         }

         std::string root_name =
            app_cfg->getProperty<std::string>(root_name_prop);

         // This has to be done before the slave connections are received so
         // that this change is included with the initial sync.
         OSG::GroupNodePtr root_node = mScene->getSceneRoot();
         OSG::beginEditCP(root_node);
            OSG::setName(root_node.node(), root_name);
         OSG::endEditCP(root_node);

         // Setup the plugins that are configured to load
         config(app_cfg);
         loadAndInitPlugins(app_cfg);
      }

      // -- Configure cluster support --- //
      jccl::ConfigElementPtr cluster_cfg =
         mConfiguration.getConfigElement(cluster_elt_type);

      if ( cluster_cfg )
      {
         // Set ourselves up as a rendering master (or not depending on the
         // configuration).
         configureNetwork(cluster_cfg);
      }
   }
}

void Viewer::preFrame()
{
   ViewerPtr myself = shared_from_this();

   // Strategy for intersection
   if (NULL != mIsectStrategy.get())
   {
      mIsectStrategy->update(myself);

      // Get the intersected object using a breadth-first search. Beginning
      // at the most shallow level of the object hierarchy, a scan is
      // performed for an object that the intersection strategy identifies as
      // being intersected.
      gmtl::Point3f cur_ip;
      SceneObjectPtr cur_obj = mIsectStrategy->findIntersection(myself, mObjects, cur_ip);

      // Save results from calling intersect on grabbable objects.
      SceneObjectPtr intersect_obj;
      SceneObjectPtr parent_obj;
      gmtl::Point3f intersect_point;
      intersect_obj = cur_obj;
      parent_obj = cur_obj;
      intersect_point = cur_ip;

      // If intersected_obj has children, then we continue the breadth-first
      // search in its children. We want to find the deepest intersected
      // object.
      std::vector<SceneObjectPtr> objs;
      while ( NULL != cur_obj.get() && intersect_obj->hasChildren() )
      {
         objs = cur_obj->getChildren();
         cur_ip.set(0.0f, 0.0f, 0.0f);
         cur_obj = mIsectStrategy->findIntersection(myself, objs, cur_ip);

         // If we intersected a child, save results
         if ( NULL != cur_obj.get() )
         {
            intersect_obj = cur_obj;
            intersect_point = cur_ip;
         }
      }

      // If we have an intersected object, make sure that the intersection is
      // valid. Yes, this is an afterthought that ought to be in the algorithm
      // above. As a breadth-first search, however, it is not clear how to
      // factor in the allowed intersection state.
      while ( NULL != intersect_obj.get() && ! intersect_obj->canIntersect() )
      {
         intersect_obj = intersect_obj->getParent();
      }

      // If the intersected object is different than the one with which the
      // wand intersected during the last frame, we need to make updates to
      // the application and scene state.
      if ( intersect_obj != mIntersectedObj )
      {
         // If we were intersecting a different object, then emit a
         // de-intersection signal for mIntersectedObj.
         if ( mIntersectedObj != NULL &&
              mIntersectedObj->getRoot() != OSG::NullFC )
         {
            OSG::FieldContainerPtr fcp = mIntersectedObj->getRoot().get();
            mEventData->mObjectDeintersectedSignal(mIntersectedObj);
            std::cout << "Deintersected: " << inf::getName(fcp) << std::endl;
         }

         // Change the intersected object to the one we found above.
         mIntersectedObj = intersect_obj;

         // If the new node of mIntersectedObj is non-NULL, then we are
         // intersecting a new object since the last frame. Emit an
         // intersection signal for this new object.
         if ( mIntersectedObj != NULL &&
              mIntersectedObj->getRoot() != OSG::NullFC)
         {
            //mIntersectSound.trigger();

            OSG::FieldContainerPtr fcp = mIntersectedObj->getRoot().get();
            mEventData->mObjectIntersectedSignal(mIntersectedObj, parent_obj, intersect_point);
            std::cout << "Intersected: " << inf::getName(fcp) << std::endl;
         }
      }
   }

   std::vector<inf::PluginPtr>::iterator i;

   // First, we update the state of each plug-in.  All plug-ins will get a
   // consistent view of the run-time state of the system before being run.
   for ( i = mPlugins.begin(); i != mPlugins.end(); ++i )
   {
      (*i)->updateState(myself);
   }

   // Then, we tell each plug-in to do its thing.  Any given plug-in may
   // change the state of the system as a result of performing its task(s).
   for ( i = mPlugins.begin(); i != mPlugins.end(); ++i )
   {
      (*i)->run(myself);
   }

   // Update the user (and navigation)
   getUser()->update(myself);
}

void Viewer::latePreFrame()
{
   //static int iter_num(0);

   OSG::Connection::Channel channel;

   // If we have networking to do then do it
   if ( NULL != mConnection )
   {
      try
      {
         OSG::UInt8 finish(false);

         mConnection->signal();
         mAspect->sendSync(*mConnection, OSG::Thread::getCurrentChangeList());
         mConnection->putValue(finish);
         sendDataToSlaves(*mConnection);
         mConnection->flush();

         while(mConnection->getSelectionCount()>0)
         {
            channel = mConnection->selectChannel();
            readDataFromSlave(*mConnection);
            mConnection->subSelection(channel);
         }

         mConnection->resetSelection();
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

   // We are using writeable change lists, so we need to clear them out
   // We do this here because it should be after anything else that the user may want to do
   OSG::Thread::getCurrentChangeList()->clearAll();
}

void Viewer::sendDataToSlaves(OSG::BinaryDataHandler& writer)
{
   OSG::UInt8 junk(false);
   writer.putValue(junk);
   float near_val, far_val;
   vrj::Projection::getNearFar(near_val, far_val);
   writer.putValue(near_val);
   writer.putValue(far_val);
}

void Viewer::readDataFromSlave(OSG::BinaryDataHandler& reader)
{
   OSG::UInt8 junk;
   reader.getValue(junk);
}

void Viewer::exit()
{
   // First we free up all the OpenSG resources that have been allocated
   // for this application.
   deallocate();

   // Then we call up to the base class implementation of this method, which
   // in turn tells OpenSG to exit.
   vrj::OpenSGApp::exit();
}

void Viewer::deallocate()
{
   if ( NULL != mAspect )
   {
      delete mAspect;
   }

   if ( NULL != mConnection )
   {
      delete mConnection;
   }

//   mPluginFactory  = inf::PluginFactoryPtr();
   mIntersectedObj = inf::SceneObjectPtr();
   mIsectStrategy  = inf::IntersectionStrategyPtr();

#if defined(_MSC_VER)
   std::vector<inf::PluginPtr>::iterator p;
   for ( p = mPlugins.begin(); p != mPlugins.end(); ++p )
   {
      (*p).reset();
   }
#else
   std::for_each(mPlugins.begin(), mPlugins.end(),
                 boost::bind((void (inf::PluginPtr::*)()) &inf::PluginPtr::reset, _1));
#endif

   mUser.reset();
   mEventData.reset();
   mScene.reset();
   mObjects.clear();
   mPlugins.clear();
   mPluginFactory.reset();

   // Output information about what is left over
   typedef std::vector<OSG::FieldContainerPtr> FieldContainerStore;
   typedef FieldContainerStore::const_iterator FieldContainerStoreConstIt;

   OSG::FieldContainerFactory* fact = OSG::FieldContainerFactory::the();
   const FieldContainerStore *pFCStore =fact->getFieldContainerStore();
   unsigned int num_types = fact->getNumTypes();

   std::vector<unsigned int> type_ids;
   OSG::FieldContainerFactory::TypeMapIterator i;
   for ( i = fact->beginTypes(); i != fact->endTypes(); ++i )
   {
      if( ((*i).second != NULL) && ((*i).second)->getPrototype() != OSG::NullFC)
      {
         unsigned int type_proto_ptr_id =
            ((*i).second)->getPrototype().getFieldContainerId();
         type_ids.push_back(type_proto_ptr_id);
         //std::cout << "Protyo ptr id: " << type_proto_ptr_id << std::endl;
      }
   }

   std::cout << "Viewer::deallocate():\n"
             << "                                    num_types: " << num_types << std::endl
             << "    Total OpenSG objects allocated (w/ types): " << pFCStore->size()
             << std::endl;

   unsigned int non_null_count(0);

   for ( unsigned int i = 0; i < pFCStore->size(); ++i )
   {
      OSG::FieldContainerPtr ptr = (*pFCStore)[i];
      if(ptr != OSG::NullFC)
      {
         non_null_count += 1;
      }
   }

   std::cout << "Remaining non-null OpenSG objects (w/o types): "
             << (non_null_count-num_types) << std::endl;

// Enable this section when you want to see the names and types of the objects
// that remain.
#if 0
   std::cout << " ---- non-null objects remaining --- " << std::endl;
   for(unsigned i=0; i<pFCStore->size(); ++i)
   {
      OSG::FieldContainerPtr ptr = (*pFCStore)[i];
      if( (std::count(type_ids.begin(), type_ids.end(), i) == 0)
           && (ptr != OSG::NullFC) )
      {
         OSG::AttachmentContainerPtr acp =
            OSG::AttachmentContainerPtr::dcast(ptr);
         const char* node_name = OSG::getName(acp);
         std::string node_name_str("<NULL>");
         if(NULL != node_name)
         {
            node_name_str = std::string(node_name);
         }

         std::cout << "   " << i << ": "
                   << "  type:" << ptr->getType().getName().str() << " id:"
                   << ptr.getFieldContainerId()
                   << " name:" << node_name_str
                   << std::endl;
      }
   }
#endif

}

/**
 * See @ref SlaveCommunicationProtocol for details of the communication.
 */
void Viewer::configureNetwork(jccl::ConfigElementPtr clusterCfg)
{
   const std::string listen_addr_prop("listen_addr");
   const std::string listen_port_prop("listen_port");
   const std::string slave_count_prop("slave_count");

   const std::string listen_addr =
      clusterCfg->getProperty<std::string>(listen_addr_prop);
   const unsigned short listen_port =
      clusterCfg->getProperty<unsigned short>(listen_port_prop);
   const unsigned int slave_count =
      clusterCfg->getProperty<unsigned int>(slave_count_prop);

   // If we have a port and at least one slave, then we need to set things up
   // for the incoming slave connections.
   if ( listen_port != 0 && slave_count != 0 )
   {
      std::cout << "Setting up remote slave network:" << std::endl;
      mAspect = new OSG::RemoteAspect();
      mConnection = OSG::ConnectionFactory::the().createGroup("StreamSock");

      // Construct the binding address to hand off to OpenSG.
      // At this point, listen_addr may be an empty string. This would give us
      // the binding address ":<listen_port>", which is fine because
      // OSG::PointSockConnection::bind() only looks at the port information.
      std::stringstream addr_stream;
      addr_stream << listen_addr << ":" << listen_port;
      std::cout << "   Attempting to bind to: " << addr_stream.str()
                << std::flush;

      // To set the IP address to which mConnection will be bound, we have to
      // do this ridiculous two-step process. If listen_addr is empty, then
      // OSG::PointSockConnection will the local host name.
      mConnection->setInterface(listen_addr);
      mConnection->bind(addr_stream.str());
      std::cout << " [OK]" << std::endl;

      mChannels.resize(slave_count);

      for ( unsigned int s = 0; s < slave_count; ++s )
      {
         std::cout << "   Waiting for slave #" << s << " to connect ..."
                   << std::flush;
         mChannels[s] = mConnection->acceptPoint();
         std::cout << "[OK]" << std::endl;
      }

      OSG::UInt8 finish(false);

      // Signal the slave nodes that we are about to send the initial sync.
      mConnection->signal();

      // Provide the slave nodes with a consistent rendering scale factor.
      mConnection->putValue(getDrawScaleFactor());
      mAspect->sendSync(*mConnection, OSG::Thread::getCurrentChangeList());
      mConnection->putValue(finish);
      mConnection->flush();

      // NOTE: We are not clearing the change list at this point
      // because that would blow away any actions taken during the
      // initialization of mScene.

      std::cout << "   All " << slave_count << " slave nodes have connected"
                << std::endl;
   }
}

void Viewer::config(jccl::ConfigElementPtr appCfg)
{
   const std::string plugin_path_prop("plugin_path");
   const std::string strategy_plugin_path_prop("strategy_plugin_path");

   const std::string iov_base_dir_tkn("IOV_BASE_DIR");

   std::vector<std::string> search_path;

   // Set up four default search paths:
   //    1. Relative path to './plugins'
   //    2. IOV_BASE_DIR/lib/IOV/plugins
   //    3. Relative path to './plugins/grab'
   //    4. IOV_BASE_DIR/lib/IOV/plugins/grab
   search_path.push_back("plugins");
   search_path.push_back("plugins/grab");

   std::string iov_base_dir;
   if ( vpr::System::getenv(iov_base_dir_tkn, iov_base_dir).success() )
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

      fs::path def_strategy_path = iov_base_path / "lib/IOV/plugins/grab";

      if ( fs::exists(def_strategy_path) )
      {
         std::string def_search_path =
            def_strategy_path.native_directory_string();
         std::cout << "Setting default IOV intersection strategy plug-in path: " << def_search_path
                   << std::endl;
         search_path.push_back(def_search_path);
      }
      else
      {
         std::cerr << "Default IOV intersection strategy plug-in path does not exist: "
                   << def_strategy_path.native_directory_string()
                   << std::endl;
      }
   }


   // Add paths from the application configuration
   const unsigned int num_paths(appCfg->getNum(plugin_path_prop));

   for ( unsigned int i = 0; i < num_paths; ++i )
   {
      std::string dir = appCfg->getProperty<std::string>(plugin_path_prop, i);
      search_path.push_back(vpr::replaceEnvVars(dir));
   }

   // Add paths from the application configuration.
   const unsigned int num_plugin_paths(appCfg->getNum(strategy_plugin_path_prop));
   for ( unsigned int i = 0; i < num_plugin_paths; ++i )
   {
      std::string dir = appCfg->getProperty<std::string>(strategy_plugin_path_prop, i);
      search_path.push_back(vpr::replaceEnvVars(dir));
   }

   mPluginFactory->init(search_path);
}

void Viewer::loadAndInitPlugins(jccl::ConfigElementPtr appCfg)
{
   std::cout << "Viewer: Loading plugins" << std::endl;

   const std::string plugin_prop("plugin");
   const std::string isect_strategy_prop("isect_strategy");
   const unsigned int num_plugins(appCfg->getNum(plugin_prop));

   for ( unsigned int i = 0; i < num_plugins; ++i )
   {
      std::string plugin_name =
         appCfg->getProperty<std::string>(plugin_prop, i);

      try
      {
         std::cout << "   Loading plugin: " << plugin_name << " .... ";
         inf::PluginCreator<inf::Plugin>* creator =
            mPluginFactory->getPluginCreator<inf::Plugin>(plugin_name);

         if ( NULL != creator )
         {
            inf::PluginPtr plugin = creator->createPlugin();
            // Initialize the plugin, and configure it.
            plugin->init(shared_from_this());
            plugin->setFocused(shared_from_this(), true);
            mPlugins.push_back(plugin);
            std::cout << "[OK]" << std::endl;
         }
         else
         {
            std::cout << "[ERROR]\n   Plug-in '" << plugin_name
                      << "' has a NULL creator!" << std::endl;
         }
      }
      catch (std::runtime_error& ex)
      {
         std::cout << "[FAILED]\n   WARNING: Failed to load plug-in '"
                   << plugin_name << "': " << ex.what() << std::endl;
      }
   }

   mIsectStrategyName = appCfg->getProperty<std::string>(isect_strategy_prop);

   if ( ! mIsectStrategyName.empty() )
   {
      // Build the IntersectionStrategy instance.
      try
      {
         IOV_STATUS << "   Loading intersection strategy plug-in '"
                    << mIsectStrategyName << "' ..." << std::flush;
         inf::PluginCreator<inf::IntersectionStrategy>* creator =
            mPluginFactory->getPluginCreator<inf::IntersectionStrategy>(
               mIsectStrategyName
            );

         if ( NULL != creator )
         {
            mIsectStrategy = creator->createPlugin();
            mIsectStrategy->init(shared_from_this());
            IOV_STATUS << "[OK]" << std::endl;
         }
         else
         {
            IOV_STATUS << "[ERROR]\nWARNING: No creator for strategy plug-in "
                       << mIsectStrategyName << std::endl;
         }
      }
      catch (std::runtime_error& ex)
      {
         IOV_STATUS << "[ERROR]\nWARNING: Failed to load strategy plug-in "
                    << mIsectStrategyName << ":\n" << ex.what() << std::endl;
      }
   }
}

void Viewer::addObject(SceneObjectPtr obj)
{
   mObjects.push_back(obj);
}

void Viewer::removeObject(SceneObjectPtr obj)
{
   object_list_t::iterator found
      = std::find(mObjects.begin(), mObjects.end(), obj);

   if (mObjects.end() != found)
   {
      mObjects.erase(found);
   }
}

}
