#ifndef VIEWPLATFORM_H
#define VIEWPLATFORM_H

#include <OpenSG/VRJ/Viewer/NavStrategy.h>
#include <gmtl/Matrix.h>

namespace inf
{
/**
 * This class holds the position of the virtual environment in the virtual world.
 * It is the sort of "magic carpet" that the user is moving around on.
 * The view platform corresponds to the physical display system that the
 * user is inside or using.
 *
 * See User for a description of the coordinate systems used.
 */
class ViewPlatform
{
public:
   ViewPlatform()
   {;}

private:
   /* The current positon of the platform in the virtual world. vw_M_plat */
   gmtl::Matrix44f   mCurPos;

   /**
    * @directed
    */
   NavStrategy * lnkNavStrategy;
};

}

#endif
