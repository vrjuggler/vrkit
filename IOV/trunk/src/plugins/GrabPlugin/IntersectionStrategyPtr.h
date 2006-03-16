// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_INTERSECTION_STRATEGY_PTR_H_
#define _INF_INTERSECTION_STRATEGY_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
namespace inf
{
   class IntersectionStrategy;
   typedef boost::shared_ptr<IntersectionStrategy> IntersectionStrategyPtr;
   typedef boost::weak_ptr<IntersectionStrategy> IntersectionStrategyWeakPtr;
}

#endif /*_INF_INTERSECTION_STRATEGY_PTR_H_*/
