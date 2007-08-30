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

#include <OpenSG/OSGSimpleAttachments.h>

#include <vrkit/Scene.h>

namespace vrkit
{

ScenePtr Scene::create()
{
   ScenePtr new_scene(new Scene);
   return new_scene;
}

Scene::~Scene()
{
   std::map<vpr::GUID, SceneDataPtr>::iterator d;
   for ( d = mSceneData.begin(); d != mSceneData.end(); ++d )
   {
      (*d).second.reset();
   }
}

ScenePtr Scene::init()
{
   mSceneRoot     = OSG::GroupNodePtr::create();
   mDecoratorRoot = OSG::GroupNodePtr::create();
   mTransformRoot = OSG::TransformNodePtr::create();

   OSG::setName(mDecoratorRoot.node(), "Decorator Root");
   OSG::setName(mTransformRoot.node(), "Transform Root");

   OSG::beginEditCP(mSceneRoot);
      mSceneRoot.node()->addChild(mDecoratorRoot.node());
      mSceneRoot.node()->addChild(mTransformRoot.node());
   OSG::endEditCP(mSceneRoot);

   return shared_from_this();
}

// NOTE: This implementation is based on Java's java.util.Collection.put()
// method.
SceneDataPtr Scene::setSceneData(const vpr::GUID& key, SceneDataPtr value)
{
   SceneDataPtr old_value;

   std::map<vpr::GUID, SceneDataPtr>::iterator i = mSceneData.find(key);
   if ( i != mSceneData.end() )
   {
      old_value = mSceneData[key];
   }

   mSceneData[key] = value;

   return old_value;
}

SceneDataPtr Scene::removeSceneData(const vpr::GUID& key)
{
   SceneDataPtr old_value;
   std::map<vpr::GUID, SceneDataPtr>::iterator i = mSceneData.find(key);

   if ( i != mSceneData.end() )
   {
      old_value = (*i).second;
      mSceneData.erase(i);
   }

   return old_value;
}

}
