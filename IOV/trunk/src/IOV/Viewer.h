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

#ifndef _INF_VIEWER_H
#define _INF_VIEWER_H

#include <IOV/Config.h>

#include <vector>
#include <boost/enable_shared_from_this.hpp>

#include <OpenSG/OSGConnection.h>
#include <OpenSG/OSGBinaryDataHandler.h>

#include <vpr/DynLoad/Library.h>
#include <jccl/Config/ConfigElementPtr.h>

#include <vrj/vrjParam.h>

#if __VJ_version >= 2003011
#  include <vrj/Draw/OpenSG/App.h>
#else
#  include <vrj/Draw/OpenSG/OpenSGApp.h>
#endif

#include <IOV/ViewerPtr.h>

#include <IOV/EventDataPtr.h>
#include <IOV/Grab/IntersectionStrategyPtr.h>
#include <IOV/PluginPtr.h>
#include <IOV/UserPtr.h>
#include <IOV/ScenePtr.h>
#include <IOV/SceneObjectPtr.h>
#include <IOV/AbstractPluginPtr.h>
#include <IOV/PluginRegistryPtr.h>

#include <IOV/Scene.h>
#include <IOV/Configuration.h>


namespace OSG
{
   class RemoteAspect;
   class GroupConnection;
}

namespace inf
{

#if __VJ_version >= 2003011
typedef vrj::opensg::App OpenSGApp;
#else
typedef vrj::OpenSGApp OpenSGApp;
#endif

/**
 * Main viewer class.
 * This class controls the rest of the classes in the system.
 *
 * It is an OpenSG VRJ application object that has been customized
 * and extended to support a "standard" usage scenario
 * across a wide-range of applications.  In effect it provides
 * a framework for VRJ OpenSG applications.
 *
 * The viewer class supports a slave communication protocol
 * for clustering.  See @ref SlaveCommunicationProtocol
 */
class IOV_CLASS_API Viewer
   : public OpenSGApp
   , public boost::enable_shared_from_this<Viewer>
{
protected:
   Viewer();

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

   /**
    * Intitializes the context and informs plug-ins of same.
    *
    * @note Derived classes must call base class implementation.
    */
   virtual void contextInit();

   /**
    * Called at the beginning of each frame.
    * Updates the plugins and then updates the scene graph over the network.
    *
    * @note Derived class implementations \em MUST call up to this method.
    */
   virtual void preFrame();

   /**
    * Called after cluster data sync and after preFrame is complete.
    * We use it to send out OpenSG scene graph updates to the other nodes.
    *
    * @post OpenSG data (change list) has been sent to all remote nodes.
    *       Local change list has been cleared (after send).
    */
   virtual void latePreFrame();

   /**
    * @note Derived classes must call base class implementation.
    */
   virtual void contextPreDraw();

   /**
    * @note Derived classes must call base class implementation.
    */
   virtual void draw();

   /**
    * @note Derived classes must call base class implementation.
    */
   virtual void contextPostDraw();

   /**
    * Called at the end of each frame.
    *
    * @note Derived class implementations \em MUST call up to this method.
    */
   virtual void postFrame()
   {;}

   /** Return the root of the associated scene. */
   ScenePtr getSceneObj()
   {
      return mScene;
   }

   /**
    * Returns the context-specific render action for this viewer. This method
    * may only be called when an OpenGL context is active.
    *
    * @since 0.28.0
    */
   OSG::RenderAction* getRenderAction();

   /**
    * @note Derived classes must call base class implementation.
    */
   virtual void contextClose();

   /**
    * @note Derived classes must call base class implementation.
    */
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

   /**
    * Return the plugin factory being used by the core viewer object.
    *
    * @since 0.36
    */
   inf::PluginRegistryPtr getPluginRegistry()
   {
      return mPluginRegistry;
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

   /** @name Scene Object Management */
   //@{
   typedef std::vector<SceneObjectPtr> object_list_t;

   /**
    * Returns the current collection of registered scene objects. Children
    * of the nodes in the returned list are considered to be registered.
    *
    * @note This method was made public in IOV 0.26.2.
    */
   const object_list_t& getObjects() const
   {
      return mObjects;
   }

   /**
    * Registers the given scene object for intersection testing.
    *
    * @post \p obj not in the list of scene objects registered for intersection
    *       testing.
    *
    * @param obj The object to register for intersetion testing.
    *
    * @note This method was made public in IOV 0.26.5.
    */
   void addObject(SceneObjectPtr obj);

   /**
    * Removes the identified object from the list scene objects registered for
    * intersection testing. If \p obj is not currently registered, this method
    * has no effect.
    *
    * @post \p obj is not in the list of scene objects registered for
    *       intersection testing.
    *
    * @param obj The object to un-register for intersetion testing.
    *
    * @note This method was made public in IOV 0.26.5.
    */
   void removeObject(SceneObjectPtr obj);
   //@}

protected:
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

   void processDeps(const std::vector<inf::AbstractPluginPtr>& deps);

   void addPlugin(inf::PluginPtr plugin);

   void config(jccl::ConfigElementPtr appCfg);

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

   /** @name Plug-in Registry */
   //@{
   /**
    * The plug-in registry instance. In general, all entities in the
    * application should be using this instance so as to keep the knowledge
    * of loaded and instantiated plug-ins centralized.
    */
   inf::PluginRegistryPtr mPluginRegistry;
   //@}

   /** List of plugins managed by the viewer.
   * @link association
   * @supplierCardinality 0..**/
   /*# Plugin lnkPlugin; */
   std::vector<PluginPtr> mPlugins;

   /** The configuration for the system (and the viewer). */
   inf::Configuration  mConfiguration;

   /** @name Intersection Strategy */
   //@{
   std::vector<std::string> mStrategyPluginPath;
   IntersectionStrategyPtr mIsectStrategy;
   std::string mIsectStrategyName;
   SceneObjectPtr mIntersectedObj;
   //@}

   object_list_t mObjects;

protected:
   EventDataPtr mEventData;
};

}


#endif
