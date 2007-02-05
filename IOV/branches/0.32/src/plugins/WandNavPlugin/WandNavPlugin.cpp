// Copyright (C) Infiscape Corporation 2005-2007

#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#endif

#include <string.h>
#include <sstream>
#include <algorithm>
#include <vector>

#include <gmtl/Matrix.h>
#include <gmtl/MatrixOps.h>
#include <gmtl/Quat.h>
#include <gmtl/QuatOps.h>
#include <gmtl/Vec.h>
#include <gmtl/VecOps.h>
#include <gmtl/Xforms.h>
#include <gmtl/Generate.h>

#include <vpr/Util/Assert.h>

#include <IOV/InterfaceTrader.h>
#include <IOV/Viewer.h>
#include <IOV/PluginCreator.h>
#include <IOV/WandInterface.h>
#include <IOV/User.h>
#include <IOV/Util/Exceptions.h>
#include <IOV/Status.h>
#include <IOV/StatusPanel.h>
#include <IOV/StatusPanelPlugin.h>

#include "WandNavPlugin.h"


static inf::PluginCreator<inf::Plugin> sPluginCreator(
   &inf::WandNavPlugin::create, "Wand Navigator Plug-in"
);

extern "C"
{

/** @name Plug-in Entry Points */
//@{
IOV_PLUGIN_API(void) getPluginInterfaceVersion(vpr::Uint32& majorVer,
                                               vpr::Uint32& minorVer)
{
   majorVer = INF_PLUGIN_API_MAJOR;
   minorVer = INF_PLUGIN_API_MINOR;
}

IOV_PLUGIN_API(inf::PluginCreatorBase*) getCreator()
{
   return &sPluginCreator;
}
//@}

}

namespace inf
{

WandNavPlugin::WandNavPlugin()
   : mLastFrameTime(0, vpr::Interval::Sec)
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

PluginPtr WandNavPlugin::init(ViewerPtr viewer)
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
                            IOV_LOCATION);
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
         IOV_STATUS << "WARNING: Config element '" << elt->getName()
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
         throw PluginException(msg.str(), IOV_LOCATION);
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
   mForwardBtn.configButtons(elt->getProperty<std::string>(for_btn_prop));
   mReverseBtn.configButtons(elt->getProperty<std::string>(rev_btn_prop));
   mRotateBtn.configButtons(elt->getProperty<std::string>(rot_btn_prop));
   mModeBtn.configButtons(elt->getProperty<std::string>(mode_btn_prop));
   mResetBtn.configButtons(elt->getProperty<std::string>(reset_btn_prop));

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

void WandNavPlugin::focusChanged(inf::ViewerPtr viewer)
{
   inf::ScenePtr scene = viewer->getSceneObj();
   StatusPanelPluginDataPtr status_panel_data =
      scene->getSceneData<StatusPanelPluginData>();

   // We can only navigate when we have focus.
   mCanNavigate = isFocused();

   if ( ! mCanNavigate )
   {
      mVelocity = 0.0f;

      if ( status_panel_data->mStatusPanelPlugin )
      {
         inf::StatusPanel& panel =
            status_panel_data->mStatusPanelPlugin->getPanel();

         if ( mForwardBtn.isConfigured() )
         {
            // The button numbers in mForwardBtn are zero-based, but we would
            // like them to be one-based in the status panel display.
            std::vector<int> btns(transformButtonVec(mForwardBtn.getButtons()));
            panel.removeControlText(btns, mForwardText);
         }

         if ( mReverseBtn.isConfigured() )
         {
            // The button numbers in mReverseBtn are zero-based, but we would
            // like them to be one-based in the status panel display.
            std::vector<int> btns(transformButtonVec(mReverseBtn.getButtons()));
            panel.removeControlText(btns, mReverseText);
         }

         if ( mRotateBtn.isConfigured() )
         {
            // The button numbers in mRotateBtn are zero-based, but we would
            // like them to be one-based in the status panel display.
            std::vector<int> btns(transformButtonVec(mRotateBtn.getButtons()));
            panel.removeControlText(btns, mRotateText);
         }

         if ( mModeBtn.isConfigured() )
         {
            // The button numbers in mModeBtn are zero-based, but we would like
            // them to be one-based in the status panel display.
            std::vector<int> btns(transformButtonVec(mModeBtn.getButtons()));
            panel.removeControlText(btns, mModeText);
         }

         if ( mResetBtn.isConfigured() )
         {
            // The button numbers in mResetBtn are zero-based, but we would
            // like them to be one-based in the status panel display.
            std::vector<int> btns(transformButtonVec(mResetBtn.getButtons()));
            panel.removeControlText(btns, mResetText);
         }
      }
   }
   else
   {
      if ( status_panel_data->mStatusPanelPlugin )
      {
         inf::StatusPanel& panel =
            status_panel_data->mStatusPanelPlugin->getPanel();

         if ( mForwardBtn.isConfigured() )
         {
            std::vector<int> btns(transformButtonVec(mForwardBtn.getButtons()));
            if ( ! panel.hasControlText(btns, mForwardText) )
            {
               panel.addControlText(btns, mForwardText);
            }
         }

         if ( mReverseBtn.isConfigured() )
         {
            std::vector<int> btns(transformButtonVec(mReverseBtn.getButtons()));
            if ( ! panel.hasControlText(btns, mReverseText) )
            {
               panel.addControlText(btns, mReverseText);
            }
         }

         if ( mRotateBtn.isConfigured() )
         {
            std::vector<int> btns(transformButtonVec(mRotateBtn.getButtons()));
            if ( ! panel.hasControlText(btns, mRotateText) )
            {
               panel.addControlText(btns, mRotateText);
            }
         }

         if ( mModeBtn.isConfigured() )
         {
            std::vector<int> btns(transformButtonVec(mModeBtn.getButtons()));
            if ( ! panel.hasControlText(btns, mModeText) )
            {
               panel.addControlText(btns, mModeText);
            }
         }

         if ( mResetBtn.isConfigured() )
         {
            std::vector<int> btns(transformButtonVec(mResetBtn.getButtons()));
            if ( ! panel.hasControlText(btns, mResetText) )
            {
               panel.addControlText(btns, mResetText);
            }
         }
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
      if ( mForwardBtn.test(mWandInterface, gadget::Digital::ON) )
      {
         mVelocity += mAcceleration;
      }
      else if ( mReverseBtn.test(mWandInterface, gadget::Digital::ON) )
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
      if ( mModeBtn.test(mWandInterface, gadget::Digital::TOGGLE_ON) )
      {
         nav_state = NONE;
         mNavMode = (mNavMode == WALK ? FLY : WALK);
         IOV_STATUS << "Mode: " << (mNavMode == WALK ? "Walk" : "Fly")
                   << std::endl;
      }
      // If the accelerate button and the rotate button are pressed together,
      // then reset to the starting point translation and rotation.
      else if ( mResetBtn.test(mWandInterface, gadget::Digital::ON) )
      {
         nav_state = RESET;
      }
      else if ( mRotateBtn.test(mWandInterface, gadget::Digital::ON) )
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

struct IncValue
{
   int operator()(int v)
   {
      return v + 1;
   }
};

std::vector<int> WandNavPlugin::transformButtonVec(const std::vector<int>& btns)
{
   std::vector<int> result(btns.size());
   IncValue inc;
   std::transform(btns.begin(), btns.end(), result.begin(), inc);
   return result;
}

}
