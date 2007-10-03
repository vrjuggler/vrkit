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

#ifndef _VRKIT_DYNAMIC_SCENE_OBJECT_H_
#define _VRKIT_DYNAMIC_SCENE_OBJECT_H_

#include <vrkit/Config.h>

#include <vector>
#include <boost/function.hpp>

#include <OpenSG/OSGAction.h>
#include <OpenSG/OSGDynamicVolume.h>
#include <OpenSG/OSGTransform.h>

#include <vrkit/SceneObject.h>
#include <vrkit/DynamicSceneObjectPtr.h>


namespace vrkit
{

/** \class DynamicSceneObject DynamicSceneObject.h vrkit/DynamicSceneObject.h
 *
 * A scene object whose children and parent are determined on the fly when
 * they are requested.
 *
 * @note This class was refactored in version 0.46.0 to be much more general
 *       than its previous incarnation, which only supported the node core
 *       type OSG::Transform. The old version is now called
 *       vrkit::DynamicSceneObjectTransform.
 *
 * @since 0.18.0
 */
class VRKIT_CLASS_API DynamicSceneObject
   : public SceneObject
{
protected:
   DynamicSceneObject();

public:
   virtual ~DynamicSceneObject();

   static DynamicSceneObjectPtr create()
   {
      return DynamicSceneObjectPtr(new DynamicSceneObject());
   }

   /**
    * Initializes this dynamic scene object. This object will be available for
    * grab-and-move interaction if and only if the given scene graph node has
    * a core of type OSG::Transform or if \p makeMoveable is true.
    *
    * @post If \p makeMoveable is true, then \p node has a core of type
    *       OSG::Transform. If \p node did not already have a core of that
    *       type, then \p node has a single child whose children are core type
    *       are that of \p node prior to invocation of this method.
    *
    * @param node         The OpenSG node with a transform core that is the
    *                     root of the scene graph sub-tree represented by this
    *                     dynamic scene object.
    * @param predicate    The predicate (a callable that takes a single
    *                     OSG::NodePtr parameter and returns a bool) that is
    *                     used to determine the parent/child relationships of
    *                     scene graph nodes on the fly. This same predicate
    *                     is used for all descendent and ancestor discovery.
    * @param makeMoveable Indicates whether this scene object and all
    *                     dynamically discovered children should be made
    *                     moveable. If true, then \p node will be examined to
    *                     determine if it has a core of type OSG::Transform.
    *                     If it does not, then a new core is created for it,
    *                     and the old core is handed off to a new child of
    *                     \p node that in turn has as its children the
    *                     original children of \p node.
    *
    * @return This object is returned as a shared pointer.
    *
    * @since 0.46.0
    */
   DynamicSceneObjectPtr init(OSG::NodePtr node,
                              boost::function<bool (OSG::NodePtr)> predicate,
                              const bool makeMoveable = true);

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
    * Indicates whether this composite object has a parent. This
    * implementation always returns false.
    *
    * @return false is always returned by this implementation.
    */
   virtual bool hasParent();

   /**
    * Returns the parent of this composite object. This implementation always
    * returns a null pointer.
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
    * Indicates whether this composite object has any children. This
    * implementation always returns false.
    *
    * @return false is always returned by this implementation.
    */
   virtual bool hasChildren();

   /**
    * Returns the number of children of this composite object. This
    * implementation always returns 0.
    *
    * @return 0 is always returned by this implementation.
    */
   virtual unsigned int getChildCount();

   /**
    * Adds the given child to this composite object's collection of children.
    * This implementation does nothing.
    *
    * @param child The child to add.
    */
   virtual void addChild(SceneObjectPtr child);

   /**
    * Removes the given child from the collection of children. This
    * implementation does nothing.
    *
    * @param child The child to remove.
    *
    * @note Subclasses may choose to throw an exception if \p child is
    *       invalid.
    */
   virtual void removeChild(SceneObjectPtr child);

   /**
    * Removes the identified child from the collection of children. This
    * implementation does nothing.
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
    * This implementation always returns a null pointer.
    *
    * @param childIndex The index of the child to return from within the
    *                   collection of children.
    *
    * @note Subclasses may choose to throw an exception if \p childIndex is
    *       invalid.
    */
   virtual SceneObjectPtr getChild(const unsigned int childIndex);

   /**
    * Returns the children of this node in a vector. This method builds up
    * the vector using getChildCount() and getChild(). This implementation
    * should be sufficient in general, but this method can be overridden
    * if necessary. One likely case when it would make sense (for performance
    * reasons) to override this method would be when the collection of
    * children held by a scene object is already maintained as a vector.
    *
    * @return A vector containing all the children of this node.
    */
   virtual std::vector<SceneObjectPtr> getChildren();
   //@}

private:
   OSG::Action::ResultE enter(OSG::NodePtr& node);

   OSG::NodeRefPtr      mRootNode;      /**< Root node of this scene object. */
   OSG::TransformRefPtr mTransformCore;

   /** @name Data for Child Discovery */
   //@{
   boost::function<bool (OSG::NodePtr)> isSceneObject;
   bool                                 mMakeMoveable;
   //@}

   std::vector<SceneObjectPtr> mChildren;      /**< Child scene objects. */
};

}


#endif /* _VRKIT_DYNAMIC_SCENE_OBJECT_H_ */
