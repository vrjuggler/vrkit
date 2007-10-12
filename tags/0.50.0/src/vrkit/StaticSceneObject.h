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

#ifndef _VRKIT_STATIC_SCENE_OBJECT_H_
#define _VRKIT_STATIC_SCENE_OBJECT_H_

#include <vrkit/Config.h>

#include <vector>

#include <OpenSG/OSGDynamicVolume.h>
#include <OpenSG/OSGTransform.h>

#include <vrkit/StaticSceneObjectPtr.h>
#include <vrkit/SceneObject.h>


namespace vrkit
{

/** \class StaticSceneObject StaticSceneObject.h vrkit/StaticSceneObject.h
 *
 * A scene object whose children and parent are determined statically through
 * construction of a tree of scene objects. This construction is performed
 * using the addChild() method.
 *
 * @since 0.18.0
 */
class VRKIT_CLASS_API StaticSceneObject
   : public SceneObject
{
protected:
   StaticSceneObject();

public:
   virtual ~StaticSceneObject();

   static StaticSceneObjectPtr create()
   {
      return StaticSceneObjectPtr(new StaticSceneObject());
   }

   /**
    * Initializes this static scene object.
    *
    * @param node The OpenSG node with a transform core that is the root of
    *             the scene graph sub-tree represented by this static scene
    *             object.
    *
    * @return This object is returned as a shared pointer.
    */
   virtual StaticSceneObjectPtr init(OSG::TransformNodePtr node);

   /**
    * Return the dynamic volume that bounds the object.
    */
   virtual OSG::DynamicVolume& getVolume(const bool update = false);

   /**
    * Move to the specified location.
    */
   virtual void moveTo(const OSG::Matrix& matrix);

   /**
    * Get the position of the scene object.
    */
   OSG::Matrix getPos();

   /**
    * Return the root OpenSG node for this object.
    */
   OSG::NodeRefPtr getRoot();

   /** @name Composite construction and query interface. */
   //@{
   /**
    * Indicates whether this composite object has a parent.
    */
   virtual bool hasParent();

   /**
    * Returns the parent of this composite object.
    */
   virtual SceneObjectPtr getParent();

   /**
    * Sets the parent for this scene object.
    *
    * @post \c mParent == \p parent.
    *
    * @param parent The new parent for this scene object.
    */
   void setParent(SceneObjectPtr parent);

   /**
    * Indicates whether this composite object has any children.
    */
   virtual bool hasChildren();

   /**
    * Returns the number of children of this composite object.
    */
   virtual unsigned int getChildCount();

   /**
    * Adds the given child to this composite object's collection of children.
    *
    * @param child The child to add.
    */
   virtual void addChild(SceneObjectPtr child);

   /**
    * Removes the given child from the collection of children.
    *
    * @param child The child to remove.
    *
    * @note Subclasses may choose to throw an exception if \p child is
    *       invalid.
    */
   virtual void removeChild(SceneObjectPtr child);

   /**
    * Removes the identified child from the collection of children.
    *
    * @param childIndex The index of the child to remove within the collection
    *                   of children.
    *
    * @note Subclasses may choose to throw an exception if \p childIndex is
    *       invalid.
    */
   virtual void removeChild(const unsigned int childIndex);

   /**
    * Returns the child at the given index within the collection of children.
    *
    * @param childIndex The index of the child to return from within the
    *                   collection of children.
    *
    * @note Subclasses may choose to throw an exception if \p childIndex is
    *       invalid.
    */
   virtual SceneObjectPtr getChild(const unsigned int childIndex);

   /**
    * Returns the children of this node in a vector. This implementation
    * should be sufficient in general, but this method can be overridden
    * if necessary.
    *
    * @return A vector containing all the children of this node.
    */
   virtual std::vector<SceneObjectPtr> getChildren();
   //@}

protected:
   OSG::TransformNodePtr              mTransformNode;
   OSG::DynamicVolume                 mEmptyVolume;
   SceneObjectWeakPtr           mParent;
   std::vector<SceneObjectPtr>  mChildren;
};

}


#endif /* _VRKIT_STATIC_SCENE_OBJECT_H_ */
