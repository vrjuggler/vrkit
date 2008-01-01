// vrkit is (C) Copyright 2005-2008
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

#include <vrkit/SceneObject.h>


namespace vrkit
{

const OSG::UInt32 SceneObject::ISECT_MASK(128);

SceneObject::~SceneObject()
{
   /* Do nothing. */ ;
}

SceneObject::SceneObject()
   : mCanIntersect(true)
   , mGrabbable(true)
{
   mEmptyVolume.setEmpty();
}

void SceneObject::wandEntered()
{
   /* Do nothing */;
}

void SceneObject::wandExited()
{
   /* Do nothing */;
}

void SceneObject::wandPressed()
{
   /* Do nothing */;
}

void SceneObject::wandReleased()
{
   /* Do nothing */;
}

void SceneObject::wandMoved()
{
   /* Do nothing */;
}

bool SceneObject::canIntersect() const
{
   return mCanIntersect;
}

void SceneObject::setAllowIntersect(const bool allowIntersect)
{
   const bool old_intersect(mCanIntersect);
   mCanIntersect = allowIntersect;

   if ( old_intersect != allowIntersect )
   {
      mIntersectStateChanged(shared_from_this());
   }
}

bool SceneObject::isGrabbable() const
{
   return mGrabbable;
}

void SceneObject::setGrabbable(const bool grabbable)
{
   const bool old_grabbable(mGrabbable);
   mGrabbable = grabbable;

   if ( old_grabbable != grabbable )
   {
      mGrabbableStateChanged(shared_from_this());
   }
}

OSG::DynamicVolume& SceneObject::getVolume(const bool update)
{
   return mEmptyVolume;
}

void SceneObject::moveTo(const OSG::Matrix& matrix)
{
   /* Do nothing */;
}

OSG::Matrix SceneObject::getPos()
{
   return OSG::Matrix::identity();
}

bool SceneObject::hasParent()
{
   return false;
}

SceneObjectPtr SceneObject::getParent()
{
   return SceneObjectPtr();
}

void SceneObject::setParent(SceneObjectPtr parent)
{
   // Do nothing
}

bool SceneObject::hasChildren()
{
   return false;
}

unsigned int SceneObject::getChildCount()
{
   return 0;
}

void SceneObject::addChild(SceneObjectPtr child)
{
   // Do nothing
}

void SceneObject::removeChild(SceneObjectPtr child)
{
   // Do nothing
}

void SceneObject::removeChild(const unsigned int childIndex)
{
   // Do nothing
}

SceneObjectPtr SceneObject::getChild(const unsigned int childIndex)
{
   return SceneObjectPtr();
}

std::vector<SceneObjectPtr> SceneObject::getChildren()
{
   return std::vector<SceneObjectPtr>();
}

void SceneObject::setNodeTravMask(OSG::NodePtr node)
{
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor ne(node, OSG::Node::TravMaskFieldMask);
#endif
   node->setTravMask(node->getTravMask() & ~ISECT_MASK);
}

}
