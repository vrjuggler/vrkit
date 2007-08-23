// Copyright (C) Infiscape Corporation 2005-2007

#include <algorithm>

#include <OpenSG/OSGNodeCore.h>

#include <IOV/Util/CoreTypePredicate.h>


namespace inf
{

struct NodeCoreValidator
{
   bool operator()(OSG::FieldContainerType* t)
   {
      return ! t->isNodeCore();
   }
};

CoreTypePredicate::
CoreTypePredicate(const std::vector<OSG::FieldContainerType*>& coreTypes)
   : mCoreTypes(coreTypes)
{
   // Remove any and all field container types from mCoreTypes that are not
   // for node cores.
   NodeCoreValidator ncv;
   mCoreTypes.erase(std::remove_if(mCoreTypes.begin(), mCoreTypes.end(), ncv),
                    mCoreTypes.end());
}

struct TypeComparator
{
   TypeComparator(const OSG::FieldContainerType& sourceType)
      : mSourceType(sourceType)
   {
      /* Do nothing. */ ;
   }

   bool operator()(OSG::FieldContainerType* t)
   {
      return mSourceType == *t || mSourceType.isDerivedFrom(*t);
   }

   const OSG::FieldContainerType& mSourceType;
};

bool CoreTypePredicate::operator()(OSG::NodePtr node)
{
   if ( OSG::NullFC == node || OSG::NullFC == node->getCore() )
   {
      return false;
   }

   TypeComparator tc(node->getCore()->getType());
   typedef std::vector<OSG::FieldContainerType*>::iterator iter_type;
   iter_type i = std::find_if(mCoreTypes.begin(), mCoreTypes.end(), tc);
   return i != mCoreTypes.end();
}

}
