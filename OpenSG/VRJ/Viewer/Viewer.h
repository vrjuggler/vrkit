#ifndef VIEWER_H
#define VIEWER_H

#include <boost/enable_shared_from_this.hpp>

#include <OpenSG/VRJ/Viewer/ViewerPtr.h>

#include <OpenSG/VRJ/Viewer/PluginPtr.h>
#include <OpenSG/VRJ/Viewer/UserPtr.h>
#include <OpenSG/VRJ/Viewer/ScenePtr.h>

#include <vector>
#include <Scene.h>


namespace inf
{

   class Viewer;
   typedef boost::shared_ptr<Viewer>   ViewerPtr;

/**
 * Main viewer class.
 * This class controls the rest of the classes in the system.
 * It is called by the VRJ application object.
 *
 * (It may actually be a VRJ app object in the future)
 */
class Viewer : public boost::enable_shared_from_this<Viewer>
{
public:
   static ViewerPtr create()
   {
      ViewerPtr new_viewer(new Viewer);
      return new_viewer;
   }

   virtual ~Viewer()
   {;}

   /** Initialize the viewer
    * @post: All objects managed by the viewer are initialized
    *        and setup.
    */
   virtual void init();

   virtual void preFrame()
   {;}

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
