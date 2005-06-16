// Copyright (C) Infiscape Corporation 2005

#include <IOV/Scene.h>

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

// NOTE: This implementation is based on Java's java.util.Collection.put()
// method.
inf::SceneDataPtr Scene::setSceneData(const vpr::GUID& key,
                                      inf::SceneDataPtr value)
{
   inf::SceneDataPtr old_value;

   std::map<vpr::GUID, inf::SceneDataPtr>::iterator i = mSceneData.find(key);
   if ( i != mSceneData.end() )
   {
      old_value = mSceneData[key];
   }

   mSceneData[key] = value;

   return old_value;
}

inf::SceneDataPtr Scene::removeSceneData(const vpr::GUID& key)
{
   inf::SceneDataPtr old_value;
   std::map<vpr::GUID, inf::SceneDataPtr>::iterator i = mSceneData.find(key);

   if ( i != mSceneData.end() )
   {
      old_value = (*i).second;
      mSceneData.erase(i);
   }

   return old_value;
}

}
