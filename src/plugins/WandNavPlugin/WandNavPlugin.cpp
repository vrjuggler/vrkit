// vrkit is (C) Copyright 2005-2011
//    by Allen Bierbaum, Aron Bierbuam, Patrick Hartling, and Daniel Shipton
//
// This file is part of vrkit.
//
// vrkit is free software; you can redistribute it and/or modify it under the
// terms of the GNU Lesser General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// vrkit is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
// more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#endif

#include <string.h>
#include <sstream>
#include <algorithm>
#include <boost/bind.hpp>
#include <boost/assign/list_of.hpp>

#include <gmtl/Matrix.h>
#include <gmtl/MatrixOps.h>
#include <gmtl/Quat.h>
#include <gmtl/QuatOps.h>
#include <gmtl/Vec.h>
#include <gmtl/VecOps.h>
#include <gmtl/Xforms.h>
#include <gmtl/Generate.h>

#include <vpr/Util/Assert.h>

#include <vrkit/Scene.h>
#include <vrkit/InterfaceTrader.h>
#include <vrkit/Viewer.h>
#include <vrkit/WandInterface.h>
#include <vrkit/User.h>
#include <vrkit/Status.h>
#include <vrkit/Version.h>
#include <vrkit/scenedata/StatusPanelData.h>
#include <vrkit/plugin/Creator.h>
#include <vrkit/plugin/Info.h>
#include <vrkit/exceptions/PluginException.h>

#include "WandNavPlugin.h"


using namespace boost::assign;

static const vrkit::plugin::Info sInfo(
   "com.infiscape", "WandNavPlugin",
   list_of(VRKIT_VERSION_MAJOR)(VRKIT_VERSION_MINOR)(VRKIT_VERSION_PATCH)
);
static vrkit::plugin::Creator<vrkit::viewer::Plugin> sPluginCreator(
   boost::bind(&vrkit::WandNavPlugin::create, sInfo)
);

extern "C"
{

/** @name Plug-in Entry Points */
//@{
VRKIT_PLUGIN_API(const vrkit::plugin::Info*) getPluginInfo()
{
   return &sInfo;
}

VRKIT_PLUGIN_API(void) getPluginInterfaceVersion(vpr::Uint32& majorVer,
                                                 vpr::Uint32& minorVer)
{
   majorVer = VRKIT_PLUGIN_API_MAJOR;
   minorVer = VRKIT_PLUGIN_API_MINOR;
}

VRKIT_PLUGIN_API(vrkit::plugin::CreatorBase*) getCreator()
{
   return &sPluginCreator;
}
//@}

}

namespace vrkit
{

WandNavPlugin::WandNavPlugin(const plugin::Info& info)
   : nav::Strategy(info)
   , mLastFrameTime(0, vpr::Interval::Sec)
   , mCanNavigate(false)
   , mVelocity(0.0f)
   , mMaxVelocity(0.5f)
   , mAcceleration(0.005f)
   , mRotationSensitivity(0.5f)
   , mNavMode(WALK)
   , mForwardText("Forward")
   , mReverseText("Reverse")
   , mRotateText("Rotate")
   , mModeText("Toggle Walk/Fly")
   , mResetText("Reset")
{
   mCanNavigate = isFocused();
}

viewer::PluginPtr WandNavPlugin::init(ViewerPtr viewer)
{
   InterfaceTrader& if_trader = viewer->getUser()->getInterfaceTrader();
   mWandInterface = if_trader.getWandInterface();

   // Configure
   std::string elt_type_name = getElementType();
   jccl::ConfigElementPtr cfg_elt =
      viewer->getConfiguration().getConfigElement(elt_type_name);

   if ( ! cfg_elt )
   {
      throw PluginException("WandNavPlugin not find its configuration.",
                            VRKIT_LOCATION);
   }

   // Configure it
   config(cfg_elt);

   return shared_from_this();
}

bool WandNavPlugin::config(jccl::ConfigElementPtr elt)
{
   const std::string for_btn_prop("forward_button_nums");
   const std::string rev_btn_prop("reverse_button_nums");
   const std::string rot_btn_prop("rotate_button_nums");
   const std::string mode_btn_prop("nav_mode_button_nums");
   const std::string reset_btn_prop("reset_button_nums");
   const std::string initial_mode_prop("initial_mode");
   const std::string rotation_sensitivity("rotation_sensitivity");
   const unsigned int req_cfg_version(3);

   vprASSERT(elt->getID() == getElementType() &&
             "Got unexpected config element type");

   // Check for correct version of plugin configuration
   if ( elt->getVersion() < req_cfg_version )
   {
      // Version 3 is backwards compatible with version 2.
      if ( elt->getVersion() == 2 )
      {
         VRKIT_STATUS << "WARNING: Config element '" << elt->getName()
                      << "', version " << elt->getVersion()
                      << ", for the Wand Nav Plug-in is out of date!\n"
                      << "         The current config element version is "
                      << req_cfg_version << ". Use VRJConfig to update."
                      << std::endl;
      }
      // Version 1 is incompatible.
      else
      {
         std::stringstream msg;
         msg << "Configuration of WandNavPlugin failed.  Required config "
             << "element version is " << req_cfg_version << ", but element '"
             << elt->getName() << "' is version " << elt->getVersion();
         throw PluginException(msg.str(), VRKIT_LOCATION);
      }
   }

   float max_velocity = elt->getProperty<float>("max_velocity");
   float accel        = elt->getProperty<float>("acceleration");
   float decel        = elt->getProperty<float>("deceleration");
   mIsDecelerationEnabled = elt->getProperty<bool>("enable_deceleration");
   mRotationSensitivity = elt->getProperty<float>(rotation_sensitivity);

   if ( max_velocity > 0.0f )
   {
      setMaximumVelocity(max_velocity);
   }

   if ( accel > 0.0f )
   {
      setAcceleration(accel);
   }

   if ( decel > 0.0f )
   {
      setDeceleration(decel);
   }

   // Get the buttons for navigation.
   mForwardBtn.configure(elt->getProperty<std::string>(for_btn_prop),
                         mWandInterface);
   mReverseBtn.configure(elt->getProperty<std::string>(rev_btn_prop),
                         mWandInterface);
   mRotateBtn.configure(elt->getProperty<std::string>(rot_btn_prop),
                        mWandInterface);
   mModeBtn.configure(elt->getProperty<std::string>(mode_btn_prop),
                      mWandInterface);
   mResetBtn.configure(elt->getProperty<std::string>(reset_btn_prop),
                       mWandInterface);

   // Get initial mode
   unsigned int mode_id = elt->getProperty<unsigned int>(initial_mode_prop);
   if ( WandNavPlugin::WALK == mode_id )
   {
      mNavMode = WandNavPlugin::WALK;
   }
   else if ( WandNavPlugin::FLY == mode_id )
   {
      mNavMode = WandNavPlugin::FLY;
   }
   else  // Default: walk
   {
      mNavMode = WandNavPlugin::WALK;
   }

   return true;
}

void WandNavPlugin::focusChanged(ViewerPtr viewer)
{
   ScenePtr scene = viewer->getSceneObj();
   StatusPanelDataPtr status_panel_data =
      scene->getSceneData<StatusPanelData>();

   // We can only navigate when we have focus.
   mCanNavigate = isFocused();

   if ( ! mCanNavigate )
   {
      mVelocity = 0.0f;

      if ( mForwardBtn.isConfigured() )
      {
         status_panel_data->removeControlText(mForwardBtn.toString(),
                                              mForwardText);
      }

      if ( mReverseBtn.isConfigured() )
      {
         status_panel_data->removeControlText(mReverseBtn.toString(),
                                              mReverseText);
      }

      if ( mRotateBtn.isConfigured() )
      {
         status_panel_data->removeControlText(mRotateBtn.toString(),
                                              mRotateText);
      }

      if ( mModeBtn.isConfigured() )
      {
         status_panel_data->removeControlText(mModeBtn.toString(),
                                              mModeText);
      }

      if ( mResetBtn.isConfigured() )
      {
         status_panel_data->removeControlText(mResetBtn.toString(),
                                              mResetText);
      }
   }
   else
   {
      if ( mForwardBtn.isConfigured() )
      {
         status_panel_data->addControlText(mForwardBtn.toString(),
                                           mForwardText, 1);
      }

      if ( mReverseBtn.isConfigured() )
      {
         status_panel_data->addControlText(mReverseBtn.toString(),
                                           mReverseText, 1);
      }

      if ( mRotateBtn.isConfigured() )
      {
         status_panel_data->addControlText(mRotateBtn.toString(), mRotateText,
                                           1);
      }

      if ( mModeBtn.isConfigured() )
      {
         status_panel_data->addControlText(mModeBtn.toString(), mModeText, 1);
      }

      if ( mResetBtn.isConfigured() )
      {
         status_panel_data->addControlText(mResetBtn.toString(), mResetText,
                                           1);
      }
   }
}

void WandNavPlugin::updateNav(ViewerPtr viewer, ViewPlatform& viewPlatform)
{
   NavState nav_state(NONE);

   if ( isFocused() )
   {
      vprASSERT(mWandInterface.get() != NULL && "No valid wand interface");

      // Update velocity
      if ( mForwardBtn() )
      {
         mVelocity += mAcceleration;
      }
      else if ( mReverseBtn() )
      {
         mVelocity -= mAcceleration;
      }
      else if ( mVelocity != 0.0f )
      {
         if ( mIsDecelerationEnabled )
         {
            if( fabs(mVelocity) <= mDeceleration )
            {
               mVelocity = 0.0f;
            }
            else if ( mVelocity > 0.0f )
            {
               mVelocity -= mDeceleration;
            }
            else
            {
               mVelocity += mDeceleration;
            }
         }
         else
         {
            mVelocity = 0.0f;
         }
      }

      // Restrict velocity range to [0.0,max_vel].
      if ( mVelocity < -mMaxVelocity )
      {
         mVelocity = -mMaxVelocity;
      }
      if ( mVelocity > mMaxVelocity )
      {
         mVelocity = mMaxVelocity;
      }

      // Swap the navigation mode if the mode switching button was toggled on.
      if ( mModeBtn() )
      {
         nav_state = NONE;
         mNavMode = (mNavMode == WALK ? FLY : WALK);
         VRKIT_STATUS << "Mode: " << (mNavMode == WALK ? "Walk" : "Fly")
                      << std::endl;
      }
      // If the accelerate button and the rotate button are pressed together,
      // then reset to the starting point translation and rotation.
      else if ( mResetBtn() )
      {
         nav_state = RESET;
      }
      else if ( mRotateBtn() )
      {
         nav_state = ROTATE;
      }
      else
      {
         nav_state = TRANSLATE;
      }
   }

   // Perform the work of navigating.
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

   if ( mCanNavigate )
   {
      gmtl::Matrix44f cur_pos = viewPlatform.getCurPos();
      const float scale_factor(viewer->getDrawScaleFactor());

      switch ( nav_state )
      {
         // Do nothing if so directed.
         case NONE:
            break;
         // Handle reset.
         case RESET:
            mVelocity = 0.0f;
            gmtl::identity(cur_pos);
            break;
         // Handle rotations.
         case ROTATE:
            {
               const gmtl::Vec3f y_axis(0.0, 1.0, 0.0);

               gmtl::Matrix44f rot_mat(
                  mWandInterface->getWandPos()->getData(scale_factor)
               );
               gmtl::setTrans(rot_mat, gmtl::Vec3f(0.0f, 0.0f, 0.0f));

               if ( gmtl::MAT_IDENTITY44F != rot_mat )
               {
                  float y_rot = gmtl::makeYRot(rot_mat);
                  gmtl::Matrix44f goal_mat =
                     gmtl::make<gmtl::Matrix44f>(gmtl::AxisAnglef(y_rot,
                                                                  y_axis));
                  gmtl::Quatf goal_quat = gmtl::make<gmtl::Quatf>(goal_mat);

                  gmtl::Quatf source_quat;
                  gmtl::Quatf slerp_quat;
                  gmtl::slerp(slerp_quat, delta_sec * mRotationSensitivity,
                              source_quat, goal_quat);

                  gmtl::Matrix44f rot_xform;
                  gmtl::set(rot_xform, slerp_quat);
                  gmtl::postMult(cur_pos, rot_xform);
               }
            }
            break;
         // Travel in model.
         case TRANSLATE:
            {
               // - Find forward direction of wand (negative z)
               // - Translate along that direction
               // Get the wand matrix
               // - Translation is in the real world (virtual platform)
               //   coordinate system
               gmtl::Matrix44f wand_mat(
                  mWandInterface->getWandPos()->getData(scale_factor)
               );
               gmtl::Vec3f z_dir(0.0f, 0.0f, -mVelocity);
               gmtl::Vec3f trans_delta = z_dir * delta_sec;
               gmtl::Vec3f trans = wand_mat * trans_delta;

               gmtl::Matrix44f trans_mat;
               gmtl::setTrans(trans_mat, trans);

               // vw_M_vp = vw_M_vp * vp_M_vp'
               cur_pos = cur_pos * trans_mat;

               // If we are in walk mode, we have to clamp the Y translation
               // value to the "ground."
               if ( mNavMode == WALK )
               {
                  // Remember that GMTL matrices are column-major.
                  cur_pos[1][3] = 0.0f;
               }
            }
            break;
         default:
            vprASSERT(false && "Bad value for nav_state");
            break;
      }

      viewPlatform.setCurPos(cur_pos);
   }
}

void WandNavPlugin::setMaximumVelocity(const float maxVelocity)
{
   mMaxVelocity = maxVelocity;
}

void WandNavPlugin::setAcceleration(const float acceleration)
{
   mAcceleration = acceleration;
}

void WandNavPlugin::setDeceleration(const float deceleration)
{
   mDeceleration = deceleration;
}

}
