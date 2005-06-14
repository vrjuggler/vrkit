#ifndef VIEWER_H
#define VIEWER_H

#include <IOV/Config.h>

#include <vector>
#include <boost/enable_shared_from_this.hpp>

#include <OpenSG/OSGConnection.h>
#include <OpenSG/OSGBinaryDataHandler.h>

#include <jccl/Config/ConfigElementPtr.h>
#include <vrj/Draw/OpenSG/OpenSGApp.h>

#include <IOV/ViewerPtr.h>

#include <IOV/PluginPtr.h>
#include <IOV/UserPtr.h>
#include <IOV/ScenePtr.h>
#include <IOV/PluginFactoryPtr.h>

#include <IOV/Scene.h>
#include <IOV/Configuration.h>


namespace OSG
{
   class RemoteAspect;
   class GroupConnection;
}

namespace inf
{

/**
 * Main viewer class.
 * This class controls the rest of the classes in the system.
 * It is called by the VRJ application object.
 *
 * (It may actually be a VRJ app object in the future)
 *
 * The viewer class supports a slave communication protocol
 * for clustering.  See @ref SlaveCommunicationProtocol
 */
class IOV_CLASS_API Viewer
   : public vrj::OpenSGApp
   , public boost::enable_shared_from_this<Viewer>
{
public:
   static ViewerPtr create()
   {
      ViewerPtr new_viewer(new Viewer);
      return new_viewer;
   }

   virtual ~Viewer();

   /** Initialize (build) the viewer
    * @post: All objects managed by the viewer are initialized
    *        and setup.  (including the plugins)
    * @note: Derived class implementations <b>MUST</b> call up to this method.
    */
   virtual void init();

   /** Intitialize the context.
    * @note: Derived classes must call base class implementation.
    */
   virtual void contextInit()
   {
      vrj::OpenSGApp::contextInit();
   }

   /** Called at the beginning of each frame.
    * Updates the plugins and then updates the scene graph over the network.
    * @note: Derived class implementations <b>MUST</b> call up to this method.
    */
   virtual void preFrame();

   /** Called after cluster data sync and after preFrame is complete.
    * We use it to send out OpenSG scene graph updates to the other nodes.
    */
   virtual void latePreFrame();

   /** Called at the end of each frame.
    * @note: Derived class implementations <b>MUST</b> call up to this method.
    */
   virtual void postFrame()
   {;}

   /** Return the root of the associated scene. */
   ScenePtr getSceneObj()
   {
      return mScene;
   }

   virtual void exit();

public:
   UserPtr      getUser()
   { return mUser; }

   /** Return the base of our scene object's root. */
   virtual OSG::NodePtr getScene()
   {
      return getSceneObj()->getSceneRoot().node();
   }

   /** Return reference to the current configuration.
    * This can be used to add to the configuration.
    * For example:  getConfiguration().loadConfigEltFile("file.jconf")
    */
   inf::Configuration& getConfiguration()
   {
      return mConfiguration;
   }

   /** Return the plugin factory being used by the system. */
   inf::PluginFactoryPtr getPluginFactory()
   {
      return mPluginFactory;
   }

   /** Dummied init scene method.  It is pure virtual in base so we have
    * to provide an implementation.
    */
   virtual void initScene()
   {;}

public:
   /** @name Cluster app data methods.
    * These methods are used to communicate data over an OpenSG network connection
    * with the cluster slave nodes.
    * It is called as part of the cluster communication protocol in latePreFrame.
    * @note Derived classes must call up to parent class methods.
    */
   //@{
   virtual void sendDataToSlaves(OSG::BinaryDataHandler& writer);

   virtual void readDataFromSlave(OSG::BinaryDataHandler& reader);
   //@}

protected:
   Viewer();

   /**
    * Override this method to deallocate OpenSG resources when the
    * application is being removed from the VR Juggler kernel.  The
    * overriding implementation must call the base class implementation
    * as its last step.
    */
   virtual void deallocate();

private:
   /**
    * Based on the given configuration, this method determines if this
    * application object will act as a rendering master for an OpenSG
    * cluster.  If we will act as a rendering master, this method blocks
    * waiting for all the expected incoming connections.
    *
    * @post If we will act as a rendering master, mAspect and mConnection are
    *       allocated memory from the heap and will be non-NULL thereafter.
    *
    * @param appCfg     the config element for the Infiscape OpenSG viewer
    *                   application object (type infiscape_opensg_viewer).
    */
   void configureNetwork(jccl::ConfigElementPtr appCfg);

   /**
    * Loads, initializes, and configures all plug-ins identified in the given
    * config element \c appCfg.
    *
    * @param appCfg     the config element for the Infiscape OpenSG viewer
    *                   application object (type infiscape_opensg_viewer).
    *
    * @see getConfiguration()
    */
   void loadAndInitPlugins(jccl::ConfigElementPtr appCfg);

private:
   /** The user for the viewer.
   * @link association
   * @supplierCardinality 1*/
   /*# User lnkUser */
   UserPtr     mUser;

   /** The scene we are using.
   * @link association
   * @supplierCardinality 1*/
   /*# Scene lnkScene; */
   ScenePtr    mScene;

   /** @name Cluster Data Members */
   //@{
   OSG::RemoteAspect*                    mAspect;
   OSG::GroupConnection*                 mConnection;
   std::vector<OSG::Connection::Channel> mChannels;
   //@}

   /**< Plugin factory for the entire system. */
   inf::PluginFactoryPtr mPluginFactory;

   /** List of plugins managed by the viewer.
   * @link association
   * @supplierCardinality 0..**/
   /*# Plugin lnkPlugin; */
   std::vector<PluginPtr> mPlugins;

   /** The configuration for the system (and the viewer). */
   inf::Configuration  mConfiguration;
};

}


#endif
