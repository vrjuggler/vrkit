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

#include <algorithm>
#include <boost/bind.hpp>

#include <OpenSG/OSGRemoteAspect.h>
#include <OpenSG/OSGGroupConnection.h>
#include <OpenSG/OSGConnectionFactory.h>
#include <OpenSG/OSGSimpleAttachments.h>

#include <vpr/vpr.h>
#include <vpr/Util/Debug.h>
#include <vpr/Util/FileUtils.h>
#include <jccl/Config/Configuration.h>
#include <vrj/Kernel/Kernel.h>

#include <vrkit/User.h>
#include <vrkit/SceneObject.h>
#include <vrkit/Status.h>
#include <vrkit/WandInterface.h>
#include <vrkit/Version.h>
#include <vrkit/scenedata/EventData.h>
#include <vrkit/isect/Strategy.h>
#include <vrkit/viewer/Plugin.h>
#include <vrkit/plugin/Registry.h>
#include <vrkit/plugin/TypedInitRegistryEntry.h>
#include <vrkit/plugin/Helpers.h>
#include <vrkit/util/OpenSGHelpers.h>
#include <vrkit/util/Debug.h>

#include <vrkit/Viewer.h>


namespace vrkit
{

template<typename T>
void registerModule(vpr::LibraryPtr module, ViewerPtr viewer)
{
   viewer->getPluginRegistry()->addEntry(
      plugin::TypedInitRegistryEntry<T>::create(
         module, &T::validatePluginLib, boost::bind(&T::init, _1, viewer)
      )
   );
}

Viewer::Viewer()
   : OpenSGApp(NULL)
   , mAspect(NULL)
   , mConnection(NULL)
{
   mPluginRegistry = plugin::Registry::create()->init();
}

Viewer::~Viewer()
{
}

void Viewer::init()
{
   // This has to be called before OSG::osgInit(), which is done by
   // OpenSGApp::init().
   OSG::ChangeList::setReadWriteDefault();

   OpenSGApp::init();

   VRKIT_STATUS << getBanner() << std::endl;

   // Verify configuration has been loaded
   bool have_config = (mConfiguration.getAllConfigElements().size() > 0);
   if (!have_config)
   {
      std::cerr << "WARNING: No configuration files were provided to the "
                << "Viewer application!" << std::endl;
   }

   ViewerPtr myself(shared_from_this());

   // Create and initialize the user.
   mUser = User::create()->init(myself,
                                vrj::Kernel::instance()->getUsers()[0]);

   // Create and initialize the base scene object.
   mScene = Scene::create()->init();

   mEventData = mScene->getSceneData<EventData>();

   // Load the app configuration and then...
   // - Setup scene root for networking
   // - Configure the networking
   // - Load and initialize the plugins
   if ( have_config )
   {
      const std::string app_elt_type("vrkit_viewer");
      const std::string cluster_elt_type("vrkit_cluster");
      const std::string root_name_prop("root_name");

      // -- Configure core application -- //
      jccl::ConfigElementPtr app_cfg =
         mConfiguration.getConfigElement(app_elt_type);

      if ( app_cfg )
      {
         const unsigned int app_cfg_ver(4);
         if ( app_cfg->getVersion() < app_cfg_ver )
         {
            std::cerr << "WARNING: vrkit Viewer config element '"
                      << app_cfg->getName() << "' is out of date!" << std::endl
                      << "         Current config element version is "
                      << app_cfg_ver << ", but this one is version "
                      << app_cfg->getVersion() << std::endl
                      << "         vrkit configuration may fail or be "
                      << "incomplete." << std::endl;
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

   mLastFrameTime = vpr::Interval::now();
}

void Viewer::contextInit()
{
   OpenSGApp::contextInit();

   // Tell each plug-in to do its thing. Any given plug-in may change the
   // state of the system as a result of performing its context initialization
   // task(s).
   std::for_each(mPlugins.begin(), mPlugins.end(),
                 boost::bind(&viewer::Plugin::contextInit, _1,
                             shared_from_this()));
}

void Viewer::preFrame()
{
   const vpr::Interval cur_time = getUser()->getHeadProxy()->getTimeStamp();
   std::cout << "Adding sample of " << (cur_time - mLastFrameTime).secd()
             << std::endl;
   mFrameRateStats.addSample((cur_time - mLastFrameTime).secd());
   mLastFrameTime = cur_time;

   ViewerPtr myself = shared_from_this();

   // Strategy for intersection
   if ( NULL != mIsectStrategy.get() )
   {
      mIsectStrategy->update(myself);

      WandInterfacePtr wand =
         getUser()->getInterfaceTrader().getWandInterface();
      const gmtl::Matrix44f vp_M_wand(
         wand->getWandPos()->getData(getDrawScaleFactor())
      );

      gmtl::Point3f intersect_point;
      SceneObjectPtr intersect_obj =
         mIsectStrategy->findIntersection(myself, mObjects, intersect_point);

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
            vprDEBUG(vrkitDBG_VIEWER, vprDBG_VERB_LVL)
               << "De-intersected: "
               << util::getName(mIntersectedObj->getRoot())
               << std::endl << vprDEBUG_FLUSH;

            mEventData->objectDeintersected(mIntersectedObj);
         }

         // Change the intersected object to the one we found above.
         mIntersectedObj = intersect_obj;

         // If the new node of mIntersectedObj is non-NULL, then we are
         // intersecting a new object since the last frame. Emit an
         // intersection signal for this new object.
         if ( mIntersectedObj != NULL &&
              mIntersectedObj->getRoot() != OSG::NullFC)
         {
            vprDEBUG(vrkitDBG_VIEWER, vprDBG_VERB_LVL)
               << "Intersected: "
               << util::getName(mIntersectedObj->getRoot().get())
               << std::endl << vprDEBUG_FLUSH;

            mEventData->objectIntersected(mIntersectedObj, intersect_point);
         }
      }
   }

   // Tell each plug-in to do its thing. Any given plug-in may change the
   // state of the system as a result of performing its task(s).
   std::for_each(mPlugins.begin(), mPlugins.end(),
                 boost::bind(&viewer::Plugin::update, _1, myself));

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

   // We are using writeable change lists, so we need to clear them out.
   // We do this here because it should be after anything else that the user
   // may want to do.
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

void Viewer::contextPreDraw()
{
   OpenSGApp::contextPreDraw();

   // Tell each plug-in to do its thing. Any given plug-in may change the
   // state of the system as a result of performing its context pre-draw
   // task(s).
   std::for_each(mPlugins.begin(), mPlugins.end(),
                 boost::bind(&viewer::Plugin::contextPreDraw, _1,
                             shared_from_this()));
}

void Viewer::draw()
{
   OpenSGApp::draw();

   // Tell each plug-in to do its thing. Any given plug-in may change the
   // state of the system as a result of performing its rendering task(s).
   std::for_each(mPlugins.begin(), mPlugins.end(),
                 boost::bind(&viewer::Plugin::draw, _1, shared_from_this()));
}

void Viewer::contextPostDraw()
{
   OpenSGApp::contextPostDraw();

   // Tell each plug-in to do its thing. Any given plug-in may change the
   // state of the system as a result of performing its rendering context
   // post-draw task(s).
   std::for_each(mPlugins.begin(), mPlugins.end(),
                 boost::bind(&viewer::Plugin::contextPostDraw, _1,
                             shared_from_this()));
}

const Viewer::base_type::context_data& Viewer::getContextData()
{
   return *mContextData;
}

void Viewer::contextClose()
{
   OpenSGApp::contextClose();

   // Tell each plug-in to do its thing. Any given plug-in may change the
   // state of the system as a result of performing its rendering context
   // shutdown task(s).
   std::for_each(mPlugins.begin(), mPlugins.end(),
                 boost::bind(&viewer::Plugin::contextClose, _1,
                             shared_from_this()));
}

void Viewer::exit()
{
   // First we free up all the OpenSG resources that have been allocated
   // for this application.
   deallocate();

   // Then we call up to the base class implementation of this method, which
   // in turn tells OpenSG to exit.
   OpenSGApp::exit();
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

   mIntersectedObj = SceneObjectPtr();
   mIsectStrategy  = isect::StrategyPtr();

#if defined(_MSC_VER)
   typedef std::vector<viewer::PluginPtr>::iterator plugin_iter_type;
   for ( plugin_iter_type p = mPlugins.begin(); p != mPlugins.end(); ++p )
   {
      (*p).reset();
   }
#else
   std::for_each(
      mPlugins.begin(), mPlugins.end(),
      boost::bind((void (viewer::PluginPtr::*)()) &viewer::PluginPtr::reset, _1)
   );
#endif

   mObjects.clear();
   mPlugins.clear();
//   mPluginRegistry.reset();
   mUser.reset();
   mEventData.reset();
   mScene.reset();

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
      if ( (*i).second != NULL && (*i).second->getPrototype() != OSG::NullFC )
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

   // Set up default search paths:
   //
   //    1. Relative path to './plugins'
   //    2. VRKIT_BASE_DIR/lib/vrkit/plugins
   //    3. Relative path to './plugins/isect'
   //    4. VRKIT_BASE_DIR/lib/vrkit/plugins/isect
   //
   // In all of the above cases, the 'debug' subdirectory is searched first if
   // this is a debug build (i.e., when VRKIT_DEBUG is defined and _DEBUG is
   // not).

   std::vector<std::string> plugin_search_path =
      plugin::getDefaultSearchPath();
   std::vector<std::string> isect_search_path =
      plugin::getDefaultSearchPath("isect");

   // Add plug-in paths from the application configuration.
   const unsigned int num_paths(appCfg->getNum(plugin_path_prop));

   if ( num_paths > 0 )
   {
      plugin_search_path.reserve(plugin_search_path.size() + num_paths);
   }

   for ( unsigned int i = 0; i < num_paths; ++i )
   {
      std::string dir = appCfg->getProperty<std::string>(plugin_path_prop, i);
      plugin_search_path.push_back(vpr::replaceEnvVars(dir));
   }

   // Add strategy search paths from the application configuration.
   const unsigned int num_strategy_paths =
      appCfg->getNum(strategy_plugin_path_prop);

   if ( num_strategy_paths > 0 )
   {
      isect_search_path.reserve(isect_search_path.size() + num_strategy_paths);
   }

   for ( unsigned int i = 0; i < num_strategy_paths; ++i )
   {
      const std::string dir =
         appCfg->getProperty<std::string>(strategy_plugin_path_prop, i);
      isect_search_path.push_back(vpr::replaceEnvVars(dir));
   }

   ViewerPtr self = shared_from_this();
   std::vector<vpr::LibraryPtr> modules =
      plugin::findModules(plugin_search_path);
   std::for_each(modules.begin(), modules.end(),
                 boost::bind(&registerModule<viewer::Plugin>, _1, self));

   modules = plugin::findModules(isect_search_path);
   std::for_each(
      modules.begin(), modules.end(),
      boost::bind(&registerModule<isect::Strategy>, _1, self)
   );
}

void Viewer::loadAndInitPlugins(jccl::ConfigElementPtr appCfg)
{
   VRKIT_STATUS << "Viewer: Loading plug-ins" << std::endl;

   const std::string plugin_prop("plugin");
   const std::string isect_strategy_prop("isect_strategy");
   const unsigned int num_plugins(appCfg->getNum(plugin_prop));

   for ( unsigned int i = 0; i < num_plugins; ++i )
   {
      const std::string plugin_type =
         appCfg->getProperty<std::string>(plugin_prop, i);

      try
      {
         VRKIT_STATUS << "   Creating instance of plug-in type "
                      << plugin_type << " ... " << std::flush;
         std::vector<AbstractPluginPtr> deps;
         AbstractPluginPtr p = mPluginRegistry->makeInstance(plugin_type,
                                                             deps);
         processDeps(deps);

         viewer::PluginPtr cur_plugin =
            boost::dynamic_pointer_cast<viewer::Plugin>(p);

         if ( cur_plugin )
         {
            addPlugin(cur_plugin);
         }
         else
         {
            std::ostringstream msg_stream;
            msg_stream << "Invalid plug-in type '"
                       << p->getInfo().getFullName()
                       << "' given as vrkit::Viewer plug-in!";
            throw PluginException(msg_stream.str(), VRKIT_LOCATION);
         }

         VRKIT_STATUS << "[OK]" << std::endl;
      }
      catch (std::runtime_error& ex)
      {
         VRKIT_STATUS << "[FAILED]\n   WARNING: Failed to load plug-in '"
                      << plugin_type << "': " << ex.what() << std::endl;
      }
   }

   mIsectStrategyName = appCfg->getProperty<std::string>(isect_strategy_prop);

   if ( ! mIsectStrategyName.empty() )
   {
      // Build the isect::Strategy instance.
      try
      {
         VRKIT_STATUS << "   Creating instance of intersection strategy "
                      << "plug-in " << mIsectStrategyName << "' ..."
                      << std::flush;
         std::vector<AbstractPluginPtr> deps;
         AbstractPluginPtr p =
            mPluginRegistry->makeInstance(mIsectStrategyName, deps);
         processDeps(deps);

         isect::StrategyPtr s =
            boost::dynamic_pointer_cast<isect::Strategy>(p);

         if ( s )
         {
            mIsectStrategy = s;
            mIsectStrategy->init(shared_from_this());
         }
         else
         {
            std::ostringstream msg_stream;
            msg_stream << "Invalid plug-in type '"
                       << p->getInfo().getFullName()
                       << "' used for vrkit::Viewer instersection strategy!";
            throw PluginException(msg_stream.str(), VRKIT_LOCATION);
         }

         VRKIT_STATUS << "[OK]" << std::endl;
      }
      catch (std::runtime_error& ex)
      {
         VRKIT_STATUS << "[ERROR]\nWARNING: Failed to load strategy plug-in "
                      << mIsectStrategyName << ":\n" << ex.what()
                      << std::endl;
      }
   }
   else
   {
      VRKIT_STATUS
         << "NOTE: No intersection strategy has been configured.\n"
         << "      vrkit::Viewer will not test for object intersections."
         << std::endl;
   }
}

void Viewer::processDeps(const std::vector<AbstractPluginPtr>& deps)
{
   typedef std::vector<AbstractPluginPtr>::const_iterator iter_type;
   for ( iter_type d = deps.begin(); d != deps.end(); ++d )
   {
      viewer::PluginPtr cur_dep =
         boost::dynamic_pointer_cast<viewer::Plugin>(*d);
      if ( cur_dep )
      {
         addPlugin(cur_dep);
      }
   }
}

void Viewer::addPlugin(viewer::PluginPtr plugin)
{
   plugin->setFocused(shared_from_this(), true);
   mPlugins.push_back(plugin);
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
