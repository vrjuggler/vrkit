#ifndef NAVSTRATEGY_H
#define NAVSTRATEGY_H

//#include <OpenSG/VRJ/Viewer/ViewPlatform.h>
#include <OpenSG/VRJ/Viewer/ViewerPtr.h>
#include <OpenSG/VRJ/Viewer/NavStrategyPtr.h>

namespace inf
{
   class ViewPlatform;

/** Base class for navigation strategies.
 * @interface
 */
class NavStrategy
{
public:
   virtual ~NavStrategy()
   {;}

   /** Initialize the navigation strategy. */
   virtual void init(ViewerPtr viewer) = 0;

   virtual void update(ViewerPtr viewer, ViewPlatform& viewPlatform) = 0;

protected:
   NavStrategy()
   {;}
};

}

#endif
