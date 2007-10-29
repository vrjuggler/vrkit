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

#ifndef _VRKIT_UTIL_OPENSG_HELPERS_H_
#define _VRKIT_UTIL_OPENSG_HELPERS_H_

#include <OpenSG/OSGCoredNodePtr.h>
#include <OpenSG/OSGSimpleAttachments.h>


namespace vrkit
{

namespace util
{

/** \struct CoredNodePred CoredNodePred.h vrkit/util/OpenSGHelpers.h
 *
 */
struct CoredNodePred
{
   typedef bool result_type;
   bool operator()(const OSG::CoredNodePtrBase& lhs,
                   const OSG::CoredNodePtrBase& rhs) const
   {
      return lhs.node() == rhs.node();
   }
};

/** \struct CoredNode2NodePred CoredNode2NodePred.h vrkit/util/OpenSGHelpers.h
 *
 */
struct CoredNode2NodePred
{
   typedef bool result_type;
   bool operator()(const OSG::CoredNodePtrBase& lhs,
                   const OSG::NodePtr rhs) const
   {
      return lhs.node() == rhs;
   }
};

inline std::string getName(const OSG::FieldContainerPtr& fcp)
{
   OSG::AttachmentContainerPtr acp =
#if OSG_MAJOR_VERSION < 2
      OSG::AttachmentContainerPtr::dcast(fcp);
#else
      OSG::cast_dynamic<OSG::AttachmentContainerPtr>(fcp);
#endif
   std::string name_str = "<NULL>";
   if ( OSG::NullFC != acp )
   {
      const char* name = OSG::getName(acp);
      if ( NULL != name )
      {
         name_str = name;
      }
   }
   return name_str;
}

}

}


#endif /* _VRKIT_UTIL_OPENSG_HELPERS_H_ */
