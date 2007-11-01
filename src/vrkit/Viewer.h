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

#ifndef _VRKIT_VIEWER_H_
#define _VRKIT_VIEWER_H_

#include <vrkit/Config.h>

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

#include <vrkit/UserPtr.h>
#include <vrkit/ScenePtr.h>
#include <vrkit/SceneObjectPtr.h>
#include <vrkit/AbstractPluginPtr.h>
#include <vrkit/Configuration.h>
#include <vrkit/scenedata/EventDataPtr.h>
#include <vrkit/plugin/RegistryPtr.h>
#include <vrkit/isect/StrategyPtr.h>
#include <vrkit/viewer/PluginPtr.h>
#include <vrkit/ViewerPtr.h>


namespace OSG
{
   class RemoteAspect;
   class GroupConnection;
}

namespace vrkit
{

#if __VJ_version >= 2003011
typedef vrj::opensg::App OpenSGApp;
#else
typedef vrj::OpenSGApp OpenSGApp;
#endif

/** \class Viewer Viewer.h vrkit/Viewer.h
 *
 * The main application class. This class controls the rest of the components
 * in a vrkit application.
 *
 * vrkit::Viewer is an OpenSG VR Juggler application object that has been
 * customized and extended to support a "standard" usage scenario across a
 * wide range of applications. In effect it provides a framework for VR Juggler
 * OpenSG applications.
 *
 * The viewer class supports a slave communication protocol for clustering. In
 * a cluster configuration, this class is designed to act as the single server
 * application instance. See @ref SlaveCommunicationProtocol
 *
 * @see vrkit::SlaveViewer
 */
class VRKIT_CLASS_API Viewer
   : public OpenSGApp
   , public boost::enable_shared_from_this<Viewer>
{
protected:
   Viewer();

public:
   /**
    * An alias for the base class of vrkit::Viewer. Due to the change
    * from vrj::OpenSGApp to vrj::opensg::App in VR Juggler 2.3.11, it is
    * convenient to have access to this type name without having to spread
    * #ifdef's around.
    *
    * @since 0.47.1
    */
   typedef vrkit::OpenSGApp base_type;

   static ViewerPtr create()
   {
      return ViewerPtr(new Viewer());
   }

   virtual ~Viewer();

   /** @name VR Juggler Application Object Interface */
   //@{
   /**
    * Initializes (builds) this viewer instance.
    *
    * @post All objects managed by the vrkit viewer are initialized and set
    *       up (including the plug-ins).
    *
    * @note Derived class overrides \em must call this implementation.
    */
   virtual void init();

   /**
    * Intitializes the context and informs plug-ins of same.
    *
    * @note Derived class overrides \em must call this implementation.
    */
   virtual void contextInit();

   /**
    * Called at the beginning of each frame.
    * Updates the plug-ins and then updates the scene graph over the network.
    *
    * @note Derived class overrides \em must call this implementation.
    */
   virtual void preFrame();

   /**
    * Called after cluster data sync and after preFrame() is complete.
    * We use it to send out OpenSG scene graph updates to the other nodes.
    * Thus, the scene graph must be in a complete state for the current frame.
    *
    * @post OpenSG data (change list) has been sent to all remote nodes.
    *       Local change list has been cleared (after send).
    *
    * @note Derived class overrides \em must call this implementation as the
    *       \em last step. For the OpenSG 2 case, this calls
    *       OSG::commitChanges() before the change list gets sent.
    */
   virtual void latePreFrame();

   /**
    * @note Derived class overrides \em must call this implementation.
    */
   virtual void contextPreDraw();

   /**
    * @note Derived class overrides \em must call this implementation.
    */
   virtual void draw();

   /**
    * @note Derived class overrides \em must call this implementation.
    */
   virtual void contextPostDraw();

   /**
    * Called at the end of each frame.
    *
    * @note Derived class overrides \em must call this implementation.
    */
   virtual void postFrame();

   /**
    * @note Derived class overrides \em must call this implementation.
    */
   virtual void contextClose();

   /** @name VR Juggler OpenSG Application Object Interface */
   //@{
   /**
    * Dummy scene initialization method. It is pure virtual in base so we have
    * to provide an implementation.
    */
   virtual void initScene();

   /** Returns the base of our scene object's root. */
   virtual OSG::NodePtr getScene();
   //@}

   /**
    * @note Derived class overrides \em must call this implementation.
    */
   virtual void exit();
   //@}

   /** Returns the central, singular vrkit user representation. */
   const UserPtr getUser() const
   {
      return mUser;
   }

   /** Returns the central, singular vrkit "scene" interface. */
   const ScenePtr getSceneObj() const
   {
      return mScene;
   }

   /**
    * Returns the context-specific data for this viewer. This method may only
    * be called when an OpenGL context is active.
    *
    * @note This method replaced vrkit::Viewer::getRenderAction().
    *
    * @since 0.47.1
    */
   const base_type::context_data& getContextData();

   /**
    * Returns the reference to the current configuration. This can be used to
    * add to the configuration.
    *
    * \code
    * getConfiguration().loadConfigEltFile("file.jconf");
    * \endcode
    */
   Configuration& getConfiguration()
   {
      return mConfiguration;
   }

   /**
    * Return the plug-in factory being used by the core viewer object.
    *
    * @since 0.36
    */
   const plugin::RegistryPtr getPluginRegistry() const
   {
      return mPluginRegistry;
   }

   /** @name Cluster Application Data Interface
    *
    * These methods are used to communicate data over an OpenSG network
    * connection with the cluster slave nodes. It is called as part of the
    * cluster communication protocol in latePreFrame.
    *
    * @note Derived classes \em must call up to parent class methods.
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
    * @note This method was made public in vrkit 0.26.2.
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
    * @note This method was made public in vrkit 0.26.5.
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
    * @note This method was made public in vrkit 0.26.5.
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
    * @param appCfg     The config element for the vrkit application object
    *                   (type vrkit_viewer).
    */
   void configureNetwork(jccl::ConfigElementPtr appCfg);

   /**
    * Loads, initializes, and configures all plug-ins identified in the given
    * config element \c appCfg.
    *
    * @param appCfg     The config element for the vrkit application object
    *                   (type vrkit_viewer).
    *
    * @see getConfiguration()
    */
   void loadAndInitPlugins(jccl::ConfigElementPtr appCfg);

   void processDeps(const std::vector<AbstractPluginPtr>& deps);

   void addPlugin(viewer::PluginPtr plugin);

   void config(jccl::ConfigElementPtr appCfg);

private:
   /** The user for the viewer. */
   UserPtr     mUser;

   /** The scene we are using. */
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
   plugin::RegistryPtr mPluginRegistry;
   //@}

   /**
    * List of plug-ins managed by the viewer (implementations of
    * vrkit::viewer::Plugin).
    */
   std::vector<viewer::PluginPtr> mPlugins;

   /** The configuration for the system (and the viewer). */
   Configuration mConfiguration;

   /** @name Intersection Strategy */
   //@{
   std::vector<std::string> mStrategyPluginPath;
   isect::StrategyPtr mIsectStrategy;
   std::string mIsectStrategyName;
   SceneObjectPtr mIntersectedObj;
   //@}

   object_list_t mObjects;

protected:
   EventDataPtr mEventData;
};

}


#endif /* _VRKIT_VIEWER_H_ */
