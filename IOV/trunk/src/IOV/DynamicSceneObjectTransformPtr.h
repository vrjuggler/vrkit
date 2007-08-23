// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_DYNAMIC_SCENE_OBJECT_TRANSFORM_PTR_H_
#define _INF_DYNAMIC_SCENE_OBJECT_TRANSFORM_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class DynamicSceneObjectTransform;
   typedef boost::shared_ptr<DynamicSceneObjectTransform>
      DynamicSceneObjectTransformPtr;
   typedef boost::weak_ptr<DynamicSceneObjectTransform>
      DynamicSceneObjectWeakTransformPtr;
}

#endif
