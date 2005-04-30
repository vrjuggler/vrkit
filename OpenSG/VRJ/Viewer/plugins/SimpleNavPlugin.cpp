#include <vpr/Util/Assert.h>

#include <gmtl/Matrix.h>
#include <gmtl/MatrixOps.h>
#include <gmtl/Quat.h>
#include <gmtl/QuatOps.h>
#include <gmtl/Vec.h>
#include <gmtl/VecOps.h>
#include <gmtl/Xforms.h>
#include <gmtl/Generate.h>

#include <OpenSG/VRJ/Viewer/IOV/InterfaceTrader.h>
#include <OpenSG/VRJ/Viewer/IOV/Viewer.h>
#include <OpenSG/VRJ/Viewer/IOV/PluginCreator.h>
#include <OpenSG/VRJ/Viewer/IOV/WandInterface.h>
#include <OpenSG/VRJ/Viewer/IOV/User.h>
#include <OpenSG/VRJ/Viewer/plugins/SimpleNavPlugin.h>


static inf::PluginCreator sPluginCreator("Simple Navigator Plug-in");

extern "C"
{

/** @name Plug-in Entry Points */
//@{
IOV_PLUGIN_API(void) getPluginInterfaceVersion(vpr::Uint32& majorVer, vpr::Uint32& minorVer)
{
   majorVer = INF_PLUGIN_API_MAJOR;
   minorVer = INF_PLUGIN_API_MINOR;
}

IOV_PLUGIN_API(inf::PluginCreator*) create()
{
   sPluginCreator.setPlugin(inf::SimpleNavPlugin::create());
   return &sPluginCreator;
}
//@}

}

namespace inf
{

void SimpleNavPlugin::init(ViewerPtr viewer)
{
   InterfaceTrader& if_trader = viewer->getUser()->getInterfaceTrader();
   mWandInterface = if_trader.getWandInterface();
}

void SimpleNavPlugin::updateNav(ViewerPtr viewer, ViewPlatform& viewPlatform)
{
   vprASSERT(mWandInterface.get() != NULL && "No valid wand interface");

   const float inc_vel(0.005f);
   const float max_vel(0.5f);

   gadget::DigitalInterface& accel_button =
      mWandInterface->getButton(ACCEL_BUTTON);
   gadget::DigitalInterface& stop_button =
      mWandInterface->getButton(STOP_BUTTON);
   gadget::DigitalInterface& rotate_button =
      mWandInterface->getButton(ROTATE_BUTTON);
   gadget::DigitalInterface& mode_button =
      mWandInterface->getButton(MODE_BUTTON);

   // Update velocity
   if ( accel_button->getData() == gadget::Digital::ON )
   {
      mVelocity += inc_vel;
   }
   else if(mVelocity > 0)
   {
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

   if ( stop_button->getData() == gadget::Digital::ON )
   {
      mVelocity = 0;
   }

   // Swap the navigation mode if the mode switching button was toggled on.
   if ( mode_button->getData() == gadget::Digital::TOGGLE_ON )
   {
      mNavMode = (mNavMode == WALK ? FLY : WALK);
      std::cout << "Mode: " << (mNavMode == WALK ? "Walk" : "Fly")
                << std::endl;
   }

   gmtl::Matrix44f cur_pos = viewPlatform.getCurPos();

   // If the accelerate button and the rotate button are pressed together,
   // then reset to the starting point translation and rotation.
   if ( accel_button->getData() && rotate_button->getData() )
   {
      mVelocity = 0.0f;
      gmtl::identity(cur_pos);
   }
   // Handle rotations.
   else if ( rotate_button->getData() == gadget::Digital::ON )
   {
      gmtl::Matrix44f rot_mat(mWandInterface->getWandPos()->getData());
      gmtl::setTrans(rot_mat, gmtl::Vec3f(0.0f, 0.0f, 0.0f));

      if ( gmtl::MAT_IDENTITY44F != rot_mat )
      {
         float y_rot = gmtl::makeYRot(rot_mat);
         gmtl::Quatf goal_quat(0.0f, 1.0f, 0.0f, y_rot);

         gmtl::Quatf source_quat;
         gmtl::Quatf slerp_quat;
         // XXX: This needs to be time-based rotation.  This value of 0.005f
         // is to compensate for a high frame rate with a simple test model.
         gmtl::slerp(slerp_quat, 0.005f, source_quat, goal_quat);

         gmtl::Matrix44f rot_xform;
         gmtl::set(rot_xform, slerp_quat);
         gmtl::postMult(cur_pos, rot_xform);
      }
   }
   // Travel in model
   else
   {
      // - Find forward direction of wand (negative z)
      // - Translate along that direction
      // Get the wand matrix
      // - Translation is in the real world (virtual platform) coordinate
      //   system
      gmtl::Matrix44f wand_mat(mWandInterface->getWandPos()->getData());
      gmtl::Vec3f z_dir = gmtl::Vec3f(0.0f, 0.0f, -mVelocity);
      gmtl::Vec3f trans = wand_mat * z_dir;

      // If we are in walk mode, we have to clamp the Y translation value to
      // the "ground."
      if ( mNavMode == WALK )
      {
         trans[1] = 0.0f;
      }

      gmtl::Matrix44f trans_mat;
      gmtl::setTrans(trans_mat, trans);

      cur_pos = cur_pos * trans_mat;            // vw_M_vp = vw_M_vp * vp_M_vp'
   }

   viewPlatform.setCurPos(cur_pos);
}

}
