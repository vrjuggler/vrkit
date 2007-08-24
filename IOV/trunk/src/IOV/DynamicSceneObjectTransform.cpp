// Copyright (C) Infiscape Corporation 2005-2007

#include <boost/mpl/vector.hpp>

#include <OpenSG/OSGFieldContainerFactory.h>

#include <IOV/Util/CoreTypeSeqPredicate.h>
#include <IOV/DynamicSceneObjectTransform.h>


namespace inf
{

DynamicSceneObjectTransform::~DynamicSceneObjectTransform()
{
   /* Do nothing. */ ;
}

DynamicSceneObjectTransform::DynamicSceneObjectTransform()
   : DynamicSceneObject()
{
   /* Do nothing. */ ;
}

DynamicSceneObjectTransformPtr
DynamicSceneObjectTransform::init(OSG::TransformNodePtr node)
{
   typedef boost::mpl::vector<OSG::Transform> core_types;

   CoreTypeSeqPredicate<core_types> pred;
   DynamicSceneObjectPtr myself = DynamicSceneObject::init(node, pred, true);

   return boost::dynamic_pointer_cast<DynamicSceneObjectTransform>(myself);
}

}
