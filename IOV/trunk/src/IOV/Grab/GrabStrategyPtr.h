// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_GRAB_STRATEGY_PTR_H_
#define _INF_GRAB_STRATEGY_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class GrabStrategy;
   typedef boost::shared_ptr<GrabStrategy> GrabStrategyPtr;
   typedef boost::weak_ptr<GrabStrategy> GrabStrategyWeakPtr;
}

#endif /*_INF_GRAB_STRATEGY_PTR_H_*/
