// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_DYNAMIC_SCENE_OBJECT_TRANSFORM_H_
#define _INF_DYNAMIC_SCENE_OBJECT_TRANSFORM_H_

#include <IOV/Config.h>

#include <OpenSG/OSGTransform.h>

#include <IOV/DynamicSceneObject.h>
#include <IOV/DynamicSceneObjectTransformPtr.h>


namespace inf
{

/**
 * A scene object whose children and parent are determined on the fly when
 * they are requested.
 *
 * @since 0.46.0
 */
class IOV_CLASS_API DynamicSceneObjectTransform
   : public DynamicSceneObject
{
protected:
   DynamicSceneObjectTransform();

public:
   virtual ~DynamicSceneObjectTransform();

   static DynamicSceneObjectTransformPtr create()
   {
      return DynamicSceneObjectTransformPtr(new DynamicSceneObjectTransform());
   }

   /**
    * Initializes this dynamic scene object.
    *
    * @param node The OpenSG node with a transform core that is the root of
    *             the scene graph sub-tree represented by this dynamic scene
    *             object.
    *
    * @return This object is returned as a shared pointer.
    */
   DynamicSceneObjectTransformPtr init(OSG::TransformNodePtr node);
};

}


#endif /* _INF_DYNAMIC_SCENE_OBJECT_TRANSFORM_H_ */
