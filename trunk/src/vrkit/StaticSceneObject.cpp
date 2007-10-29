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

#include <sstream>

#include <vrkit/Exception.h>
#include <vrkit/StaticSceneObject.h>


namespace vrkit
{

StaticSceneObject::~StaticSceneObject()
{
   /* Do nothing. */ ;
}

StaticSceneObjectPtr StaticSceneObject::init(OSG::TransformNodePtr node)
{
   mTransformNode = node;
   setNodeTravMask(node.node());

   return boost::dynamic_pointer_cast<StaticSceneObject>(shared_from_this());
}

StaticSceneObject::StaticSceneObject()
{
   mEmptyVolume.setEmpty();
}

void StaticSceneObject::moveTo(const OSG::Matrix& matrix)
{
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor tnce(mTransformNode.core(), OSG::Transform::MatrixFieldMask);
#endif
   mTransformNode->setMatrix(matrix);
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

#if OSG_MAJOR_VERSION < 2
   return mTransformNode.node()->getVolume(update);
#else
   return mTransformNode.node()->editVolume(update);
#endif
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

      SceneObjectPtr myself(shared_from_this());
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
      throw Exception(msg.str(), VRKIT_LOCATION);
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
      throw Exception(msg.str(), VRKIT_LOCATION);
   }
}

std::vector<SceneObjectPtr> StaticSceneObject::getChildren()
{
   return mChildren;
}

}
