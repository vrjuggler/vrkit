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

OSG::NodeRefPtr SceneObject::getRoot()
{
   return OSG::NodeRefPtr();
}

bool SceneObject::hasParent() const
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

bool SceneObject::hasChildren() const
{
   return false;
}

unsigned int SceneObject::getChildCount() const
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
