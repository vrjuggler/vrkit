// Copyright (C) Infiscape Corporation 2005-2006

#include <IOV/SceneObject.h>

namespace inf
{

SceneObject::~SceneObject()
{
   /* Do nothing. */ ;
}

SceneObject::SceneObject()
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
