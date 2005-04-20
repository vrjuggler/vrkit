#include <vpr/Util/Assert.h>

#include <gmtl/Matrix.h>
#include <gmtl/MatrixOps.h>
#include <gmtl/Vec.h>
#include <gmtl/VecOps.h>
#include <gmtl/Xforms.h>
#include <gmtl/Generate.h>

#include <OpenSG/VRJ/Viewer/InterfaceTrader.h>
#include <OpenSG/VRJ/Viewer/Viewer.h>
#include <OpenSG/VRJ/Viewer/WandInterface.h>
#include <OpenSG/VRJ/Viewer/User.h>
#include <OpenSG/VRJ/Viewer/SimpleNavStrategy.h>


namespace inf
{

void SimpleNavStrategy::init(ViewerPtr viewer)
{
   InterfaceTrader& if_trader = viewer->getUser()->getInterfaceTrader();
   mWandInterface = if_trader.getWandInterface();
}

void SimpleNavStrategy::update(ViewerPtr viewer, ViewPlatform& viewPlatform)
{
   vprASSERT(mWandInterface.get() != NULL && "No valid wand interface");

   const float inc_vel(0.005f);
   const float max_vel(0.5f);

   // Update velocity
   if(mWandInterface->getButton(0)->getData() == gadget::Digital::ON)
   {
      mVelocity += inc_vel;
      std::cout << "vel: " << mVelocity << std::endl;
   }
   else if(mVelocity > 0)
   {
      std::cout << "vel: " << mVelocity << std::endl;
      mVelocity -= inc_vel;
   }

   // Restrict range
   if(mVelocity < 0)
   {
      mVelocity = 0;
   }
   if(mVelocity > max_vel)
   {
      mVelocity = max_vel;
   }

   if(mWandInterface->getButton(1)->getData() == gadget::Digital::ON)
   {
      mVelocity = 0;
   }

   // Travel in model
   // - Find forward direction of wand (negative z)
   // - Translate along that direction
   // Get the wand matrix
   // - Translation is in the real world (virtual platform) coordinate system
   gmtl::Matrix44f wand_mat(mWandInterface->getWandPos()->getData());
   gmtl::Vec3f z_dir = gmtl::Vec3f(0.0f, 0.0f, -mVelocity);
   gmtl::Vec3f trans = wand_mat * z_dir;

   gmtl::Matrix44f trans_mat;
   gmtl::setTrans(trans_mat, trans);

   gmtl::Matrix44f cur_pos = viewPlatform.getCurPos();
   cur_pos = cur_pos * trans_mat;                        // vw_M_vp = vw_M_vp * vp_M_vp'

   viewPlatform.setCurPos(cur_pos);
}

}
