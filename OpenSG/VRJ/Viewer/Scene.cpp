#include <OpenSG/VRJ/Viewer/Scene.h>

namespace inf
{

ScenePtr Scene::create()
{
   ScenePtr new_scene(new Scene);
   return new_scene;
}

void Scene::init()
{
   mSceneRoot     = CoredGroupPtr::create();
   mDecoratorRoot = CoredGroupPtr::create();
   mTransformRoot = CoredTransformPtr::create();

   OSG::beginEditCP(mSceneRoot);
      mSceneRoot.node()->addChild(mDecoratorRoot.node());
      mSceneRoot.node()->addChild(mTransformRoot.node());
   OSG::endEditCP(mSceneRoot);
}

}

