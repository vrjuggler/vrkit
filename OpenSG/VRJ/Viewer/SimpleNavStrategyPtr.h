#ifndef SIMPLE_NAV_STRATEGY_PTR_H_
#define SIMPLE_NAV_STRATEGY_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class SimpleNavStrategy;
   typedef boost::shared_ptr<SimpleNavStrategy> SimpleNavStrategyPtr;
   typedef boost::weak_ptr<SimpleNavStrategy> SimpleNavStrategyWeakPtr;
}

#endif

