#ifndef VIEWPLATFORM_H
#define VIEWPLATFORM_H

#include <OpenSG/VRJ/Viewer/IOV/ViewerPtr.h>

#include <gmtl/Matrix.h>
#include <gmtl/MatrixOps.h>
#include <gmtl/External/OpenSGConvert.h>

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

   /** Update the view platform (navigation) */
   void update(ViewerPtr viewer);

   /** Return the current position.
    * This is the position of the view platform in the virtual world.
    * vw_M_vp
    */
   const gmtl::Matrix44f& getCurPos() const
   {
      return mCurPos;
   }

   void setCurPos(const gmtl::Matrix44f& mat)
   {
      mCurPos = mat;
      gmtl::invert(mCurPosInv, mCurPos);
   }

   /** Get the inverse of the current position.
    * vp_M_vw
    */
   const gmtl::Matrix44f& getCurPosInv() const
   {
      return mCurPosInv;
   }

private:
   /* The current positon of the platform in the virtual world. vw_M_vp.
   * This is the positiong of the virtual platform in the virtual world.
   */
   gmtl::Matrix44f   mCurPos;

   /** Inverse matrix of the current position.
    * vp_M_vw
    */
   gmtl::Matrix44f   mCurPosInv;
};

}

#endif
