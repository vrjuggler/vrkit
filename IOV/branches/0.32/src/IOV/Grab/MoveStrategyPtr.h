// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_MOVE_STRATEGY_PTR_H_
#define _INF_MOVE_STRATEGY_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
namespace inf
{
   class MoveStrategy;
   typedef boost::shared_ptr<MoveStrategy> MoveStrategyPtr;
   typedef boost::weak_ptr<MoveStrategy> MoveStrategyWeakPtr;
}

#endif /*_INF_MOVE_STRATEGY_PTR_H_*/
