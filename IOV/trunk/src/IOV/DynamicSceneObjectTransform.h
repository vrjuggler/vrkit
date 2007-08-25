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
