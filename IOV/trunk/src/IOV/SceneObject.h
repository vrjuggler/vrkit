// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_SCENE_OBJECT_H_
#define _INF_SCENE_OBJECT_H_

#include <vector>
#include <boost/enable_shared_from_this.hpp>
#include <boost/signal.hpp>

#include <OpenSG/OSGDynamicVolume.h>
#include <OpenSG/OSGNodePtr.h>
#include <OpenSG/OSGMatrix.h>

#include <IOV/Config.h>
#include <IOV/SceneObjectPtr.h>
#include <IOV/Util/SignalProxy.h>


namespace inf
{

/**
 * Base type for objects in the scene. Such objects correspond to a subtree
 * in the scene graph but exist as a higher level concept. Exactly what defines
 * an object in the scene is up to the application programmer. One use of this
 * type is in handling grabbable objects.
 *
 * This type uses the Composite Pattern to allow for easy management of
 * parent/child relationships of scene objects. Code in IOV operatins in terms
 * of this abstract base type. Application classes can derive from this base
 * type to define custom scene objects.
 *
 * @see inf::GrabData
 *
 * @since 0.18.0
 */
class IOV_CLASS_API SceneObject
   : public boost::enable_shared_from_this<SceneObject>
{
protected:
   SceneObject();

public:
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

   /** @name Signal Accessors for Slot Connection */
   //@{
   /**
    * A signal whose slot takes the source scene object as its only parameter.
    *
    * @since 0.24.1
    */
   typedef boost::signal<void (inf::SceneObjectPtr)> self_signal_t;

   /**
    * A signal whose slot takes the source scene object and another modified
    * scene object as its parameters.
    *
    * @since 0.24.1
    */
   typedef boost::signal<void (inf::SceneObjectPtr, inf::SceneObjectPtr)>
      self_obj_signal_t;

   /**
    * Retrieves a proxy to the signal object for intersection state changes.
    *
    * @since 0.24.1
    */
   SignalProxy<self_signal_t> intersectStateChanged()
   {
      return SignalProxy<self_signal_t>(mIntersectStateChanged);
   }

   /**
    * Retrieves a proxy to the signal object for grab state changes.
    *
    * @since 0.24.1
    */
   SignalProxy<self_signal_t> grabbableStateChanged()
   {
      return SignalProxy<self_signal_t>(mGrabbableStateChanged);
   }

   /**
    * Retrieves a proxy to the signal object for child addition.
    *
    * @since 0.24.1
    */
   SignalProxy<self_obj_signal_t> childAdded()
   {
      return SignalProxy<self_obj_signal_t>(mChildAdded);
   }

   /**
    * Retrieves a proxy to the signal object for child removal.
    *
    * @since 0.24.1
    */
   SignalProxy<self_obj_signal_t> childRemoved()
   {
      return SignalProxy<self_obj_signal_t>(mChildRemoved);
   }
   //@}

protected:
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


#endif
