// Copyright (C) Infiscape Corporation 2005-2007

#include <OpenSG/OSGFieldContainerFactory.h>

#include <IOV/Util/CoreTypePredicate.h>
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
   std::vector<OSG::FieldContainerType*> xform_type(
      1,
      OSG::FieldContainerFactory::the()->findType(
         OSG::Transform::getClassTypeId()
      )
   );

   CoreTypePredicate pred(xform_type);
   DynamicSceneObjectPtr myself = DynamicSceneObject::init(node, pred, true);

   return boost::dynamic_pointer_cast<DynamicSceneObjectTransform>(myself);
}

}
