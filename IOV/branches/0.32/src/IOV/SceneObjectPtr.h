// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_SCENE_OBJECT_PTR_H_
#define _INF_SCENE_OBJECT_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
namespace inf
{
   class SceneObject;
   typedef boost::shared_ptr<SceneObject> SceneObjectPtr;
   typedef boost::weak_ptr<SceneObject> SceneObjectWeakPtr;   
}

#endif

