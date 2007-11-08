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

#ifndef _VRKIT_SCENE_OBJECT_H_
#define _VRKIT_SCENE_OBJECT_H_

#include <vector>
#include <boost/enable_shared_from_this.hpp>
#include <boost/signal.hpp>

#include <OpenSG/OSGDynamicVolume.h>
#include <OpenSG/OSGNodePtr.h>
#include <OpenSG/OSGMatrix.h>

#include <vrkit/Config.h>
#include <vrkit/SceneObjectPtr.h>
#include <vrkit/signal/Proxy.h>


namespace vrkit
{

/** \class SceneObject SceneObject.h vrkit/SceneObject.h
 *
 * Base type for objects in the scene. Such objects correspond to a subtree
 * in the scene graph but exist as a higher level concept. Exactly what defines
 * an object in the scene is up to the application programmer. One use of this
 * type is in handling grabbable objects.
 *
 * This type uses the Composite Pattern to allow for easy management of
 * parent/child relationships of scene objects. Code in vrkit operatins in
 * terms of this abstract base type. Application classes can derive from this
 * base type to define custom scene objects.
 *
 * @see vrkit::GrabData
 *
 * @since 0.18.0
 */
class VRKIT_CLASS_API SceneObject
   : public boost::enable_shared_from_this<SceneObject>
{
protected:
   SceneObject();

public:
   /**
    * The traversal mask used by vrkit for performing intersection tests with
    * OSG::IntersectAction.
    *
    * @see setNodeTravMask()
    *
    * @since 0.51.3
    *
    * @note This was originallly added in version 0.50.2 on the 0.50 branch
    *       of the Subversion repository and merged to the trunk with version
    *       0.51.3.
    */
   static const OSG::UInt32 ISECT_MASK;

   virtual ~SceneObject();

   /**
    * Return the dynamic volume that bounds the object.
    */
   virtual OSG::DynamicVolume& getVolume(const bool update = false);

   virtual void wandEntered();
   virtual void wandExited();
   virtual void wandPressed();
   virtual void wandReleased();
   virtual void wandMoved();

   /**
    * Determines if this scene object can be intersected for the purposes of
    * being grabbed (object intersection is usually done before grabbing).
    * Scene objects allow intersection by default.
    *
    * @return \c true is returned if intersection with this scene object is
    *         allowed; \c false is returned otherwise.
    *
    * @see setAllowIntersect()
    * @see isGrabbable()
    * @see setGrabbable()
    *
    * @since 0.23.3
    */
   virtual bool canIntersect() const;

   /**
    * Changes whether intersection is allowed with this scene object.
    * Disallowing intersection testing may have the side effect of preventing
    * the object from being grabbed.
    *
    * @param allowIntersect Flag indicating whether intersction with this
    *                       scene object is allowed.
    *
    * @see canIntersect()
    * @see isGrabbable()
    * @see setGrabbable()
    *
    * @since 0.23.3
    */
   virtual void setAllowIntersect(const bool allowIntersect);

   /**
    * Determines if this scene object can be grabbed. Scene objects are
    * grabbable by default.
    *
    * @return \c true is returned if grabbing of this scene object is allowed;
    *         \c false is returned otherwise.
    *
    * @see setGrabbable()
    *
    * @since 0.23.3
    */
   virtual bool isGrabbable() const;

   /**
    * Sets the grabbable state for this scene object.
    *
    * @param grabbable Flag indicating whether this scene object should be
    *                  grabbable.
    *
    * @see isGrabbable()
    *
    * @since 0.23.3
    */
   virtual void setGrabbable(const bool grabbable);

   /**
    * Move to the specified location.
    */
   virtual void moveTo(const OSG::Matrix& matrix);

   /**
    * Get the position of the scene object.
    */
   virtual OSG::Matrix getPos();

   /**
    * Return the root OpenSG node for this object.
    */
   virtual OSG::NodeRefPtr getRoot() = 0;

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
   virtual void setParent(SceneObjectPtr parent);

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
    * Returns the children of this node in a vector. This implementation
    * always returns an empty vector.
    *
    * @return An empty vector is returned.
    */
   virtual std::vector<SceneObjectPtr> getChildren();
   //@}

   /** @name Signal Accessors for Slot Connection */
   //@{
   /**
    * A signal whose slot takes the source scene object as its only parameter.
    *
    * @since 0.24.1
    */
   typedef boost::signal<void (SceneObjectPtr)> self_signal_t;

   /**
    * A signal whose slot takes the source scene object and another modified
    * scene object as its parameters.
    *
    * @since 0.24.1
    */
   typedef boost::signal<void (SceneObjectPtr, SceneObjectPtr)>
      self_obj_signal_t;

   /**
    * Retrieves a proxy to the signal object for intersection state changes.
    *
    * @since 0.24.1
    */
   signal::Proxy<self_signal_t> intersectStateChanged()
   {
      return signal::Proxy<self_signal_t>(mIntersectStateChanged);
   }

   /**
    * Retrieves a proxy to the signal object for grab state changes.
    *
    * @since 0.24.1
    */
   signal::Proxy<self_signal_t> grabbableStateChanged()
   {
      return signal::Proxy<self_signal_t>(mGrabbableStateChanged);
   }

   /**
    * Retrieves a proxy to the signal object for child addition.
    *
    * @since 0.24.1
    */
   signal::Proxy<self_obj_signal_t> childAdded()
   {
      return signal::Proxy<self_obj_signal_t>(mChildAdded);
   }

   /**
    * Retrieves a proxy to the signal object for child removal.
    *
    * @since 0.24.1
    */
   signal::Proxy<self_obj_signal_t> childRemoved()
   {
      return signal::Proxy<self_obj_signal_t>(mChildRemoved);
   }
   //@}

protected:
   /**
    * Sets the traversal mask on the given node so that it is suitable for
    * vrkit intersection testing.
    *
    * @post The traversal mask field for \p node is changed.
    *
    * @since 0.48.1
    */
   virtual void setNodeTravMask(OSG::NodePtr node);

   /** @name Intersection and Grab State */
   //@{
   bool mCanIntersect;  /**< Indicates whether intersection is allowed. */
   bool mGrabbable;     /**< Indicates whether grabbing is allowed. */
   //@}

   OSG::DynamicVolume           mEmptyVolume;

   /** @name Signals */
   //@{
   /**
    * Signal for changes to \c mCanIntersect.
    *
    * @since 0.24.1
    */
   self_signal_t mIntersectStateChanged;

   /**
    * Signal for changes to \c mGrabbable.
    *
    * @since 0.24.1
    */
   self_signal_t mGrabbableStateChanged;

   /**
    * Signal emitted when a child is added to this scene object.
    *
    * @since 0.24.1
    */
   self_obj_signal_t mChildAdded;

   /**
    * Signal emitted when a child is removed from this scene object.
    *
    * @since 0.24.1
    */
   self_obj_signal_t mChildRemoved;
   //@}
};

}


#endif /* _VRKIT_SCENE_OBJECT_H_ */
