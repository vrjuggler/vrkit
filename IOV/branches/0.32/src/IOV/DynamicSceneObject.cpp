// Copyright (C) Infiscape Corporation 2005-2007

#include <IOV/DynamicSceneObject.h>
#include <OpenSG/OSGSimpleAttachments.h>

namespace inf
{

DynamicSceneObject::~DynamicSceneObject()
{
   /* Do nothing. */ ;
}

DynamicSceneObject::DynamicSceneObject()
   : SceneObject()
{
   /* Do nothing. */ ;
}

DynamicSceneObjectPtr DynamicSceneObject::init(OSG::TransformNodePtr node)
{
   mTransformNode = node;
   OSG::UInt32 trav_mask = mTransformNode.node()->getTravMask();
   trav_mask = (trav_mask & ~128);
   OSG::beginEditCP(mTransformNode.node(), OSG::Node::TravMaskFieldMask);
      mTransformNode.node()->setTravMask(trav_mask);
   OSG::endEditCP(mTransformNode.node(), OSG::Node::TravMaskFieldMask);

   return boost::dynamic_pointer_cast<DynamicSceneObject>(shared_from_this());
}

OSG::DynamicVolume& DynamicSceneObject::getVolume(const bool update)
{
   return getRoot()->getVolume(update);
}

bool isTransform(OSG::NodePtr node)
{
   if (OSG::NullFC == node || OSG::NullFC == node->getCore())
   {
      return false;
   }

   if (node->getCore()->getType().isDerivedFrom(OSG::Transform::getClassType()))
   {
      return true;
   }
   return false;
}

void DynamicSceneObject::moveTo(const OSG::Matrix& matrix)
{
   OSG::beginEditCP(mTransformNode.core(), OSG::Transform::MatrixFieldMask);
      mTransformNode->setMatrix(matrix);
   OSG::endEditCP(mTransformNode.core(), OSG::Transform::MatrixFieldMask);
}

OSG::Matrix DynamicSceneObject::getPos()
{
   return mTransformNode->getMatrix();
}

OSG::NodeRefPtr DynamicSceneObject::getRoot()
{
   return OSG::NodeRefPtr(mTransformNode.node());
}

bool DynamicSceneObject::hasParent()
{
   return (getParent().get() != NULL);
}

SceneObjectPtr DynamicSceneObject::getParent()
{
   OSG::NodePtr parent = getRoot()->getParent();
   while (OSG::NullFC != parent)
   {
      if (isTransform(parent))
      {
         OSG::TransformNodePtr parent_node(parent);
         return DynamicSceneObject::create()->init(parent_node);
      }
      parent = parent->getParent();
   }
   return DynamicSceneObjectPtr();
}

void DynamicSceneObject::setParent(SceneObjectPtr parent)
{
   // Do nothing
}

bool DynamicSceneObject::hasChildren()
{
   return (!getChildren().empty());
}

unsigned int DynamicSceneObject::getChildCount()
{
   return (getChildren().size());
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
   return (getChildren()[childIndex]);
}

OSG::Action::ResultE DynamicSceneObject::enter(OSG::NodePtr& node)
{   
   if(node->getCore()->getType().isDerivedFrom(OSG::Transform::getClassType()))
   {
      OSG::TransformNodePtr parent_node(node);
      mChildren.push_back(DynamicSceneObject::create()->init(parent_node));
      /*
      std::string node_str("<NULL>");
      const char* node_name = OSG::getName(node);
      if (NULL != node_name)
      {
         node_str = node_name;
      }
      std::cout << "Adding node: " << node_str << std::endl;
      */
      return OSG::Action::Skip;
   }   
   return OSG::Action::Continue; 
}

std::vector<SceneObjectPtr> DynamicSceneObject::getChildren()
{
   //std::cout << "DynamicSceneObject::getChildren()" << std::endl;
   //if (mChildren.empty())
   {
   mChildren.clear();

   // Traverse over all children.
   OSG::traverse(getRoot()->getMFChildren()->getValues(), 
                 OSG::osgTypedMethodFunctor1ObjPtrCPtrRef<OSG::Action::ResultE,
                                                          DynamicSceneObject,
                                                          OSG::NodePtr>(
                                                          this, 
                                                          &DynamicSceneObject::enter));
   }
   return mChildren;
}

}
