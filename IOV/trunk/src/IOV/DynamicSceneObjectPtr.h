// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_DYNAMIC_SCENE_OBJECT_PTR_H_
#define _INF_DYNAMIC_SCENE_OBJECT_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
namespace inf
{
   class DynamicSceneObject;
   typedef boost::shared_ptr<DynamicSceneObject> DynamicSceneObjectPtr;
   typedef boost::weak_ptr<DynamicSceneObject> DynamicSceneObjectWeakPtr;
}

#endif

