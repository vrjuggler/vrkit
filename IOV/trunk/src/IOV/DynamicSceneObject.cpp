// Copyright (C) Infiscape Corporation 2005-2006

#include <IOV/DynamicSceneObject.h>

namespace inf
{

DynamicSceneObject::~DynamicSceneObject()
{
   /* Do nothing. */ ;
}

DynamicSceneObject::DynamicSceneObject()
{
   mEmptyVolume.setEmpty();
}


OSG::DynamicVolume& DynamicSceneObject::getVolume(const bool update)
{
   return mEmptyVolume;
}

OSG::NodeRefPtr DynamicSceneObject::getRoot()
{
   return OSG::NodeRefPtr();
}

bool DynamicSceneObject::hasParent() const
{
   return false;
}

SceneObjectPtr DynamicSceneObject::getParent()
{
   return DynamicSceneObjectPtr();
}

void DynamicSceneObject::setParent(SceneObjectPtr parent)
{
   // Do nothing
}

bool DynamicSceneObject::hasChildren() const
{
   return false;
}

unsigned int DynamicSceneObject::getChildCount() const
{
   return 0;
}

void DynamicSceneObject::addChild(SceneObjectPtr child)
{
   // Do nothing
}

void DynamicSceneObject::removeChild(SceneObjectPtr child)
{
   // Do nothing
}

void DynamicSceneObject::removeChild(const unsigned int childIndex)
{
   // Do nothing
}

SceneObjectPtr DynamicSceneObject::getChild(const unsigned int childIndex)
{
   // Do nothing
}

std::vector<SceneObjectPtr> DynamicSceneObject::getChildren()
{
   return std::vector<SceneObjectPtr>();
}

}
