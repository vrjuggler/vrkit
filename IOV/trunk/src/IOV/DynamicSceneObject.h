// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_DYNAMIC_SCENE_OBJECT_H_
#define _INF_DYNAMIC_SCENE_OBJECT_H_

#include <IOV/Config.h>
#include <IOV/DynamicSceneObjectPtr.h>
#include <IOV/SceneObject.h>

#include <OpenSG/OSGDynamicVolume.h>

#include <boost/enable_shared_from_this.hpp>
#include <vector>

namespace inf
{

/**
 * A scene object whose children and parent are determined on the fly when
 * they are requested.
 *
 * @since 0.18.0
 */
class IOV_CLASS_API DynamicSceneObject
   : public SceneObject
{
protected:
   DynamicSceneObject();

public:
   virtual ~DynamicSceneObject();

   /**
    * Return the dynamic volume that bounds the object.
    */
   virtual OSG::DynamicVolume& getVolume(const bool update = false);

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
   virtual bool hasParent() const;

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
   virtual bool hasChildren() const;

   /**
    * Returns the number of children of this composite object. This
    * implementation always returns 0.
    *
    * @return 0 is always returned by this implementation.
    */
   virtual unsigned int getChildCount() const;

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

protected:
   OSG::DynamicVolume           mEmptyVolume;
};

}

#endif
