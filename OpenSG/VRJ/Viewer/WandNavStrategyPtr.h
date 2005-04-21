#ifndef WAND_NAV_STRATEGY_PTR_H_
#define WAND_NAV_STRATEGY_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class WandNavStrategy;
   typedef boost::shared_ptr<WandNavStrategy> WandNavStrategyPtr;
   typedef boost::weak_ptr<WandNavStrategy> WandNavStrategyWeakPtr;
}

#endif

