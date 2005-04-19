#ifndef VIEWER_H
#define VIEWER_H

#include <vrj/Kernel/Kernel.h>
#include <vrj/Draw/OpenSG/OpenSGApp.h>

#include <boost/enable_shared_from_this.hpp>

#include <OpenSG/VRJ/Viewer/ViewerPtr.h>

#include <OpenSG/VRJ/Viewer/PluginPtr.h>
#include <OpenSG/VRJ/Viewer/UserPtr.h>
#include <OpenSG/VRJ/Viewer/ScenePtr.h>

#include <vector>
#include <Scene.h>


namespace inf
{

/**
 * Main viewer class.
 * This class controls the rest of the classes in the system.
 * It is called by the VRJ application object.
 *
 * (It may actually be a VRJ app object in the future)
 */
class Viewer : public vrj::OpenSGApp, public boost::enable_shared_from_this<Viewer>
{
public:
   static ViewerPtr create()
   {
      ViewerPtr new_viewer(new Viewer);
      return new_viewer;
   }

   virtual ~Viewer()
   {;}

   /** Initialize (build) the viewer
    * @post: All objects managed by the viewer are initialized
    *        and setup.
    * @note: Derived class implementations <b>MUST</b> call up to this method.
    */
   virtual void init();

   /** Called at the beginning of each frame.
    * @note: Derived class implementations <b>MUST</b> call up to this method.
    */
   virtual void preFrame()
   {;}

   /** Called at the end of each frame.
    * @note: Derived class implementations <b>MUST</b> call up to this method.
    */
   virtual void postFrame()
   {;}

public:
   UserPtr      getUser()
   { return mUser; }

   /** Return the root of the associated scene. */
   OSG::NodePtr getSceneRoot()
   {
      return mScene->getSceneRoot();
   }

protected:
   Viewer()
      : vrj::OpenSGApp(NULL)
   {;}

private:

   /** The user for the viewer.
   * @link association */
   /*# User lnkUser */
   UserPtr     mUser;

   /** The scene we are using.
   * @link association */
   /*# Scene lnkScene; */
   ScenePtr    mScene;

   /** List of plugins
   * @link association
   * @supplierCardinality 0..**/
   /*# Plugin lnkPlugin; */
   std::vector<PluginPtr> mPlugins;
};

}
#endif
