// Copyright (C) Infiscape Corporation 2005-2006

#include <IOV/SceneObject.h>

namespace inf
{

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

}
