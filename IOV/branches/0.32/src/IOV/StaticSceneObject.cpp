// Copyright (C) Infiscape Corporation 2005-2007

#include <IOV/StaticSceneObject.h>
#include <IOV/Util/Exceptions.h>

#include <sstream>

namespace inf
{

StaticSceneObject::~StaticSceneObject()
{
   /* Do nothing. */ ;
}

StaticSceneObjectPtr StaticSceneObject::init(OSG::TransformNodePtr node)
{
   mTransformNode = node;
   OSG::UInt32 trav_mask = mTransformNode.node()->getTravMask();
   trav_mask = (trav_mask & ~128);
   OSG::beginEditCP(mTransformNode.node(), OSG::Node::TravMaskFieldMask);
      mTransformNode.node()->setTravMask(trav_mask);
   OSG::endEditCP(mTransformNode.node(), OSG::Node::TravMaskFieldMask);

   return boost::dynamic_pointer_cast<StaticSceneObject>(shared_from_this());
}

StaticSceneObject::StaticSceneObject()
{
   mEmptyVolume.setEmpty();
}

void StaticSceneObject::moveTo(const OSG::Matrix& matrix)
{
   OSG::beginEditCP(mTransformNode.core(), OSG::Transform::MatrixFieldMask);
      mTransformNode->setMatrix(matrix);
   OSG::endEditCP(mTransformNode.core(), OSG::Transform::MatrixFieldMask);
}

OSG::Matrix StaticSceneObject::getPos()
{
   return mTransformNode->getMatrix();
}

OSG::NodeRefPtr StaticSceneObject::getRoot()
{
   return OSG::NodeRefPtr(mTransformNode.node());
}

OSG::DynamicVolume& StaticSceneObject::getVolume(const bool update)
{
   if (OSG::NullFC == mTransformNode.node())
   {
      return mEmptyVolume;
   }
   return mTransformNode.node()->getVolume(update);
}


bool StaticSceneObject::hasParent()
{
   return mParent.lock().get() != NULL;
}

SceneObjectPtr StaticSceneObject::getParent()
{
   return mParent.lock();
}

void StaticSceneObject::setParent(SceneObjectPtr parent)
{
   mParent = parent;
}

bool StaticSceneObject::hasChildren()
{
   return ! mChildren.empty();
}

unsigned int StaticSceneObject::getChildCount()
{
   return mChildren.size();
}

void StaticSceneObject::addChild(SceneObjectPtr child)
{
   // We only add a child if it points to valid data.
   if ( child.get() != NULL )
   {
      // If child already has a parent, we have to break that relationship.
      if ( child->hasParent() )
      {
         child->getParent()->removeChild(child);
      }

      inf::SceneObjectPtr myself(shared_from_this());
      child->setParent(myself);
      mChildren.push_back(child);

      mChildAdded(myself, child);
   }
}

void StaticSceneObject::removeChild(SceneObjectPtr child)
{
   std::vector<SceneObjectPtr>::iterator c = std::find(mChildren.begin(),
                                                       mChildren.end(),
                                                       child);

   if ( c != mChildren.end() )
   {
      // child no longer has a parent.
      child->setParent(SceneObjectPtr());
      mChildren.erase(c);

      mChildRemoved(shared_from_this(), child);
   }
}

void StaticSceneObject::removeChild(const unsigned int childIndex)
{
   if ( childIndex < mChildren.size() )
   {
      removeChild(mChildren[childIndex]);
   }
   else
   {
      std::ostringstream msg;
      msg << "removeChild() failed: Child index " << childIndex
          << " is not in the range [0," << mChildren.size() << ")";
      throw inf::Exception(msg.str(), IOV_LOCATION);
   }
}

SceneObjectPtr StaticSceneObject::getChild(const unsigned int childIndex)
{
   if ( childIndex < mChildren.size() )
   {
      return mChildren[childIndex];
   }
   else
   {
      std::ostringstream msg;
      msg << "getChild() failed: Child index " << childIndex
          << " is not in the range [0," << mChildren.size() << ")";
      throw inf::Exception(msg.str(), IOV_LOCATION);
   }
}

std::vector<SceneObjectPtr> StaticSceneObject::getChildren()
{
   return mChildren;
}

}
