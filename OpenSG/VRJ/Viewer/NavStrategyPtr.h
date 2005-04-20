#ifndef NAV_STRATEGY_PTR_H_
#define NAV_STRATEGY_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class NavStrategy;
   typedef boost::shared_ptr<NavStrategy> NavStrategyPtr;
   typedef boost::weak_ptr<NavStrategy> NavStrategyWeakPtr;
}

#endif

