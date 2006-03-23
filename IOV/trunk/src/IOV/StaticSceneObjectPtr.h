// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_STATIC_SCENE_OBJECT_PTR_H_
#define _INF_STATIC_SCENE_OBJECT_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
namespace inf
{
   class StaticSceneObject;
   typedef boost::shared_ptr<StaticSceneObject> StaticSceneObjectPtr;
   typedef boost::weak_ptr<StaticSceneObject> StaticSceneObjectWeakPtr;   
}

#endif

