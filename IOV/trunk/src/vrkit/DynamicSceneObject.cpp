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

#include <vrkit/DynamicSceneObject.h>


namespace vrkit
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

DynamicSceneObjectPtr DynamicSceneObject::
init(OSG::NodePtr node, boost::function<bool (OSG::NodePtr)> predicate,
     const bool makeMoveable)
{
   mRootNode     = node;
   isSceneObject = predicate;
   mMakeMoveable = makeMoveable;

   OSG::beginEditCP(mRootNode, OSG::Node::TravMaskFieldMask);
      mRootNode->setTravMask(mRootNode->getTravMask() & ~128);
   OSG::endEditCP(mRootNode, OSG::Node::TravMaskFieldMask);

   if ( makeMoveable )
   {
      // Use OSG::NodeCoreRefPtr here to ensure that the object does not get
      // destroyed if structural changes have to occur.
      OSG::NodeCoreRefPtr root_node_core(node->getCore());
      OSG::TransformPtr xform_core =
         OSG::TransformPtr::dcast(root_node_core.get());

      // If geomRoot does not currently have a transform core, then we need to
      // create a new node to hold the core of geomRoot and give geomRoot a
      // transform core. The children of geomRoot are moved to be children of
      // the newly created node, and this new node is added as the only child
      // of node. The process is done this way so that it is safe to do during
      // traversal of the OpenSG scene graph.
      if ( OSG::NullFC == xform_core )
      {
         // Step 1: Create the transform core and assign it to node.
         mTransformCore = OSG::Transform::create();
         OSG::beginEditCP(node, OSG::Node::CoreFieldMask);
            node->setCore(mTransformCore);
         OSG::endEditCP(node, OSG::Node::CoreFieldMask);

         // Step 2: Create a new node in the scene graph and give it the old
         // core from node.
         OSG::NodeRefPtr new_child(OSG::Node::create());
         OSG::beginEditCP(new_child, OSG::Node::CoreFieldMask);
            new_child->setCore(root_node_core);
         OSG::endEditCP(new_child, OSG::Node::CoreFieldMask);

         // Step 3: Move the children of node to new_child.
         const OSG::UInt32 num_children(node->getNChildren());
         OSG::beginEditCP(new_child, OSG::Node::ChildrenFieldMask);
            for ( OSG::UInt32 c = 0; c < num_children; ++c )
            {
               // NOTE: A side effect of this is that the child will be
               // removed from node. As such, using child #0 here is correct.
               new_child->addChild(node->getChild(0));
            }
         OSG::endEditCP(new_child, OSG::Node::ChildrenFieldMask);

         // Step 4: Add new_child as a child of node.
         OSG::beginEditCP(node, OSG::Node::ChildrenFieldMask);
            node->addChild(new_child);
         OSG::endEditCP(node, OSG::Node::ChildrenFieldMask);
      }

      // At this point, node has a transform core.
   }
   else
   {
      OSG::NodeCorePtr root_node_core(node->getCore());

      // If node has a core of type OSG::Transform, then this will make it
      // available for use in moving this object. Otherwise, it will
      // (re)assign mTransformCore the value of OSG::NullFC.
      mTransformCore = OSG::TransformPtr::dcast(root_node_core);
   }

   return boost::dynamic_pointer_cast<DynamicSceneObject>(shared_from_this());
}

OSG::DynamicVolume& DynamicSceneObject::getVolume(const bool update)
{
   return getRoot()->getVolume(update);
}

void DynamicSceneObject::moveTo(const OSG::Matrix& matrix)
{
   if ( OSG::NullFC != mTransformCore )
   {
      OSG::beginEditCP(mTransformCore, OSG::Transform::MatrixFieldMask);
         mTransformCore->setMatrix(matrix);
      OSG::endEditCP(mTransformCore, OSG::Transform::MatrixFieldMask);
   }
}

OSG::Matrix DynamicSceneObject::getPos()
{
   if ( OSG::NullFC != mTransformCore )
   {
      return mTransformCore->getMatrix();
   }
   else
   {
      return OSG::Matrix();
   }
}

OSG::NodeRefPtr DynamicSceneObject::getRoot()
{
   return mRootNode;
}

bool DynamicSceneObject::hasParent()
{
   return getParent().get() != NULL;
}

SceneObjectPtr DynamicSceneObject::getParent()
{
   OSG::NodePtr parent = getRoot()->getParent();
   while (OSG::NullFC != parent)
   {
      if ( isSceneObject(parent) )
      {
         return DynamicSceneObject::create()->init(parent, isSceneObject,
                                                   mMakeMoveable);
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
   if ( isSceneObject(node) )
   {
      mChildren.push_back(DynamicSceneObject::create()->init(node,
                                                             isSceneObject,
                                                             mMakeMoveable));
      return OSG::Action::Skip;
   }

   return OSG::Action::Continue;
}

std::vector<SceneObjectPtr> DynamicSceneObject::getChildren()
{
   //if (mChildren.empty())
   {
   mChildren.clear();

   // Traverse over all children.
   OSG::traverse(
      getRoot()->getMFChildren()->getValues(),
      OSG::osgTypedMethodFunctor1ObjPtrCPtrRef<
         OSG::Action::ResultE, DynamicSceneObject, OSG::NodePtr
      >(this, &DynamicSceneObject::enter));
   }

   return mChildren;
}

}
