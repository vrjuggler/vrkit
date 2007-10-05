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

#include <algorithm>

#include <OpenSG/OSGNodeCore.h>

#include <vrkit/util/CoreTypePredicate.h>


namespace vrkit
{

namespace util
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

}
