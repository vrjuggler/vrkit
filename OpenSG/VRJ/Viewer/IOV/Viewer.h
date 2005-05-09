#ifndef VIEWER_H
#define VIEWER_H

#include <OpenSG/VRJ/Viewer/IOV/Config.h>

#ifdef WIN32
#include <winsock.h>
#endif

#include <vector>
#include <boost/enable_shared_from_this.hpp>

#include <OpenSG/OSGRemoteAspect.h>
#include <OpenSG/OSGGroupConnection.h>

#include <vpr/vpr.h>
#include <vpr/DynLoad/Library.h>
#include <jccl/Config/ConfigElementPtr.h>
#include <vrj/Draw/OpenSG/OpenSGApp.h>

#include <OpenSG/VRJ/Viewer/IOV/ViewerPtr.h>

#include <OpenSG/VRJ/Viewer/IOV/PluginPtr.h>
#include <OpenSG/VRJ/Viewer/IOV/UserPtr.h>
#include <OpenSG/VRJ/Viewer/IOV/ScenePtr.h>

#include <OpenSG/VRJ/Viewer/IOV/Scene.h>


namespace inf
{

/**
 * Main viewer class.
 * This class controls the rest of the classes in the system.
 * It is called by the VRJ application object.
 *
 * (It may actually be a VRJ app object in the future)
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

   void setConfiguration(const std::string& cfgFile)
   {
      mCfgFile = cfgFile;
   }

   /** Initialize (build) the viewer
    * @post: All objects managed by the viewer are initialized
    *        and setup.
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
    * @note: Derived class implementations <b>MUST</b> call up to this method.
    */
   virtual void preFrame();

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

   void addPlugin(PluginPtr plugin);

public:
   UserPtr      getUser()
   { return mUser; }

   /** Return the base of our scene object's root. */
   virtual OSG::NodePtr getScene()
   {
      return getSceneObj()->getSceneRoot().node();
   }

   /** Dummied init scene method.  It is pure virtual in base so we have
    * to provide an implementation.
    */
   virtual void initScene()
   {;}

protected:
   Viewer()
      : vrj::OpenSGApp(NULL)
      , mAspect(NULL)
      , mConnection(NULL)
   {;}

   std::string mCfgFile;

private:
   void configureNetwork(jccl::ConfigElementPtr appCfg);

   /** The user for the viewer.
   * @link association */
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

   std::vector<vpr::LibraryPtr> mLoadedDsos;

   /** List of plugins
   * @link association
   * @supplierCardinality 0..**/
   /*# Plugin lnkPlugin; */
   std::vector<PluginPtr> mPlugins;
};

}


#endif
