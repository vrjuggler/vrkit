#ifndef SIMPLE_NAV_STRATEGY_H
#define SIMPLE_NAV_STRATEGY_H

#include <boost/enable_shared_from_this.hpp>

#include <OpenSG/VRJ/Viewer/NavStrategy.h>
#include <OpenSG/VRJ/Viewer/SimpleNavStrategyPtr.h>
#include <OpenSG/VRJ/Viewer/WandInterfacePtr.h>


namespace inf
{

class SimpleNavStrategy : public NavStrategy, public boost::enable_shared_from_this<SimpleNavStrategy>
{
public:
   static SimpleNavStrategyPtr create()
   {
      SimpleNavStrategyPtr new_strategy = SimpleNavStrategyPtr(new SimpleNavStrategy);
      return new_strategy;
   }

   virtual ~SimpleNavStrategy()
   {
      ;
   }

   virtual void init(ViewerPtr viewer);

   virtual void update(ViewerPtr viewer, ViewPlatform& viewPlatform);

protected:
   SimpleNavStrategy()
      : mVelocity(0.0f)
   {
      ;
   }

   WandInterfacePtr mWandInterface;

   float mVelocity;
};

}


#endif
