#ifndef SCENE_H
#define SCENE_H

#include <OpenSG/VRJ/Viewer/ScenePtr.h>
#include <OpenSG/OSGNode.h>
#include <OpenSG/OSGTransform.h>

#include <boost/enable_shared_from_this.hpp>

namespace inf
{

class Scene : public boost::enable_shared_from_this<Scene>
{
public:
   /** Factory method for scenes. */
   static ScenePtr create();

   virtual ~Scene()
   {;}

   virtual void init()
   {;}

   virtual OSG::NodePtr getSceneRoot()
   {
      return mSceneRoot;
   }

protected:
   Scene()
   {;}

protected:
   //              SceneRoot
   //             /        |
   //  DecoratorRoot      TransformNode
   //
   OSG::NodePtr        mSceneRoot;
   OSG::TransformPtr   mSceneTransform;
   OSG::NodePtr        mModelRoot;

   OSG::NodePtr  mLightNode;
   OSG::NodePtr  mLightBeacon;

   std::map<std::string, OSG::NodePtr>  scene_or_model_nodes;  // ???
};

}

#endif
