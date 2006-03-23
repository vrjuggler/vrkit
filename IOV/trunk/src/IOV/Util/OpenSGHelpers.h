// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_OPENSG_HELPERS_H_
#define _INF_OPENSG_HELPERS_H_

#include <OpenSG/OSGCoredNodePtr.h>

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

}

#endif /*_INF_OPENSG_HELPERS_H_*/
