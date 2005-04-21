#ifndef WAND_NAV_STRATEGY_H
#define WAND_NAV_STRATEGY_H

#include <boost/enable_shared_from_this.hpp>

#include <OpenSG/VRJ/Viewer/NavStrategy.h>
#include <OpenSG/VRJ/Viewer/WandInterfacePtr.h>

#include <OpenSG/VRJ/Viewer/WandNavStrategyPtr.h>


namespace inf
{

class WandNavStrategy
   : public NavStrategy
   , public boost::enable_shared_from_this<WandNavStrategy>
{
public:
   static WandNavStrategyPtr create()
   {
      WandNavStrategyPtr new_strategy = WandNavStrategyPtr(new WandNavStrategy);
      return new_strategy;
   }

   virtual ~WandNavStrategy()
   {
      ;
   }

   virtual void init(ViewerPtr viewer);

   virtual void update(ViewerPtr viewer, ViewPlatform& viewPlatform);

protected:
   /** Navigation mode. */
   enum NavMode
   {
      WALK,     /**< Walk (drive) mode */
      FLY       /**< Fly mode */
   };

   WandNavStrategy()
      : mVelocity(0.0f)
      , mNavMode(WALK)
      , ACCEL_BUTTON(0)
      , STOP_BUTTON(1)
      , ROTATE_BUTTON(2)
      , MODE_BUTTON(3)
   {
      ;
   }

   WandInterfacePtr mWandInterface;

   float mVelocity;
   NavMode mNavMode;

   const int ACCEL_BUTTON;
   const int STOP_BUTTON;
   const int ROTATE_BUTTON;
   const int MODE_BUTTON;
};

}


#endif
