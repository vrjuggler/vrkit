#include <vpr/Util/Assert.h>

#include <gmtl/Matrix.h>
#include <gmtl/MatrixOps.h>
#include <gmtl/Quat.h>
#include <gmtl/QuatOps.h>
#include <gmtl/Vec.h>
#include <gmtl/VecOps.h>
#include <gmtl/Xforms.h>
#include <gmtl/Generate.h>

#include <OpenSG/VRJ/Viewer/InterfaceTrader.h>
#include <OpenSG/VRJ/Viewer/Viewer.h>
#include <OpenSG/VRJ/Viewer/WandInterface.h>
#include <OpenSG/VRJ/Viewer/User.h>
#include <OpenSG/VRJ/Viewer/plugins/WandNavPlugin.h>


extern "C"
{

/** @name Plug-in Entry Points */
//@{
void getPluginInterfaceVersion(vpr::Uint32& majorVer, vpr::Uint32& minorVer)
{
   majorVer = INF_PLUGIN_API_MAJOR;
   minorVer = INF_PLUGIN_API_MINOR;
}

inf::PluginPtr create()
{
   return inf::WandNavPlugin::create();
}
//@}

}

namespace inf
{

void WandNavPlugin::init(ViewerPtr viewer)
{
   InterfaceTrader& if_trader = viewer->getUser()->getInterfaceTrader();
   mWandInterface = if_trader.getWandInterface();
}

bool WandNavPlugin::canHandleElement(jccl::ConfigElementPtr elt)
{
   return elt->getID() == getElementType();
}

bool WandNavPlugin::config(jccl::ConfigElementPtr elt)
{
   vprASSERT(elt->getID() == getElementType() &&
             "Got unexpected config element type");

   float max_velocity = elt->getProperty<float>("max_velocity");
   float accel        = elt->getProperty<float>("acceleration");

   if ( max_velocity > 0.0f )
   {
      setMaximumVelocity(max_velocity);
   }

   if ( accel > 0.0f )
   {
      setAcceleration(accel);
   }

   return true;
}

void WandNavPlugin::updateNav(ViewerPtr viewer, ViewPlatform& viewPlatform)
{
   vprASSERT(mWandInterface.get() != NULL && "No valid wand interface");

   const vpr::Interval cur_time(mWandInterface->getWandPos()->getTimeStamp());
   const vpr::Interval delta(cur_time - mLastFrameTime);
   float delta_sec(0.0f);

   // Sanity check on the frame delta.
   if ( cur_time > mLastFrameTime )
   {
      delta_sec = delta.secf();

      if ( delta_sec > 1.0f )
      {
         delta_sec = 1.0f;
      }
   }

   mLastFrameTime = cur_time;

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
      mVelocity += mAcceleration;
   }
   else if ( mVelocity > 0.0f )
   {
      mVelocity -= mAcceleration;
   }

   // Restrict velocity range to [0.0,max_vel].
   if ( mVelocity < 0.0f )
   {
      mVelocity = 0.0f;
   }
   if ( mVelocity > mMaxVelocity )
   {
      mVelocity = mMaxVelocity;
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
         gmtl::slerp(slerp_quat, delta_sec, source_quat, goal_quat);

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
      gmtl::Vec3f z_dir(0.0f, 0.0f, -mVelocity);
      gmtl::Vec3f trans_delta = z_dir * delta_sec;
      gmtl::Vec3f trans = wand_mat * trans_delta;

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

void WandNavPlugin::setMaximumVelocity(const float maxVelocity)
{
   mMaxVelocity = maxVelocity;
}

void WandNavPlugin::setAcceleration(const float acceleration)
{
   mAcceleration = acceleration;
}

}
