// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_OPENSG_HELPERS_H_
#define _INF_OPENSG_HELPERS_H_

#include <OpenSG/OSGCoredNodePtr.h>
#include <OpenSG/OSGSimpleAttachments.h>

namespace inf
{

struct CoredNodePred
{
   typedef bool result_type;
   bool operator()(const OSG::CoredNodePtrBase& lhs, const OSG::CoredNodePtrBase& rhs) const
   {
      return lhs.node() == rhs.node();
   }
};

struct CoredNode2NodePred
{
   typedef bool result_type;
   bool operator()(const OSG::CoredNodePtrBase& lhs, const OSG::NodePtr rhs) const
   {
      return lhs.node() == rhs;
   }
};

inline std::string getName(const OSG::FieldContainerPtr& fcp)
{
   OSG::AttachmentContainerPtr acp = OSG::AttachmentContainerPtr::dcast(fcp);
   std::string name_str = "<NULL>";
   if (OSG::NullFC != acp)
   {
      const char* name = OSG::getName(acp);
      if(NULL != name)
      { name_str = name; }
   }
   return name_str;
}

}

#endif /*_INF_OPENSG_HELPERS_H_*/
