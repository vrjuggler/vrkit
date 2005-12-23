// Copyright (C) Infiscape Corporation 2005

#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#endif

#include <string.h>
#include <sstream>
#include <numeric>
#include <algorithm>
#include <vector>
#include <boost/algorithm/string.hpp>

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


static inf::PluginCreator sPluginCreator(&inf::WandNavPlugin::create,
                                         "Wand Navigator Plug-in");

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

IOV_PLUGIN_API(inf::PluginCreator*) getCreator()
{
   return &sPluginCreator;
}
//@}

}

namespace inf
{

struct ButtonTest
{
   ButtonTest()
   {
   }

   bool operator()(int b0, int b1)
   {
      return true;
   }
};

WandNavPlugin::WandNavPlugin()
   : mLastFrameTime(0, vpr::Interval::Sec)
   , mCanNavigate(false)
   , mNavState(RESET)
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

void WandNavPlugin::init(ViewerPtr viewer)
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
   const unsigned int req_cfg_version(2);

   vprASSERT(elt->getID() == getElementType() &&
             "Got unexpected config element type");

   // Check for correct version of plugin configuration
   if ( elt->getVersion() < req_cfg_version )
   {
      std::stringstream msg;
      msg << "Configuration of WandNavPlugin failed.  Required config "
          << "element version is " << req_cfg_version << ", but element '"
          << elt->getName() << "' is version " << elt->getVersion();
      throw PluginException(msg.str(), IOV_LOCATION);
   }

   float max_velocity = elt->getProperty<float>("max_velocity");
   float accel        = elt->getProperty<float>("acceleration");
   mRotationSensitivity = elt->getProperty<float>(rotation_sensitivity);

   if ( max_velocity > 0.0f )
   {
      setMaximumVelocity(max_velocity);
   }

   if ( accel > 0.0f )
   {
      setAcceleration(accel);
   }

   // Get the buttons for navigation.
   configButtons(elt, for_btn_prop, mForwardBtn);
   configButtons(elt, rev_btn_prop, mReverseBtn);
   configButtons(elt, rot_btn_prop, mRotateBtn);
   configButtons(elt, mode_btn_prop, mModeBtn);
   configButtons(elt, reset_btn_prop, mResetBtn);

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

         if ( mForwardBtn.mButtonVec[0] != -1 )
         {
            // The button numbers in mForwardBtn.mButtonVec are zero-based, but
            // we would like them to be one-based in the status panel display.
            std::vector<int> btns(transformButtonVec(mForwardBtn.mButtonVec));
            panel.removeControlText(btns, mForwardText);
         }

         if ( mReverseBtn.mButtonVec[0] != -1 )
         {
            // The button numbers in mReverseBtn.mButtonVec are zero-based, but
            // we would like them to be one-based in the status panel display.
            std::vector<int> btns(transformButtonVec(mReverseBtn.mButtonVec));
            panel.removeControlText(btns, mReverseText);
         }

         if ( mRotateBtn.mButtonVec[0] != -1 )
         {
            // The button numbers in mRotateBtn.mButtonVec are zero-based, but
            // we would like them to be one-based in the status panel display.
            std::vector<int> btns(transformButtonVec(mRotateBtn.mButtonVec));
            panel.removeControlText(btns, mRotateText);
         }

         if ( mModeBtn.mButtonVec[0] != -1 )
         {
            // The button numbers in mModeBtn.mButtonVec are zero-based, but
            // we would like them to be one-based in the status panel display.
            std::vector<int> btns(transformButtonVec(mModeBtn.mButtonVec));
            panel.removeControlText(btns, mModeText);
         }

         if ( mResetBtn.mButtonVec[0] != -1 )
         {
            // The button numbers in mResetBtn.mButtonVec are zero-based, but
            // we would like them to be one-based in the status panel display.
            std::vector<int> btns(transformButtonVec(mResetBtn.mButtonVec));
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

         if ( mForwardBtn.mButtonVec[0] != -1 )
         {
            std::vector<int> btns(transformButtonVec(mForwardBtn.mButtonVec));
            if ( ! panel.hasControlText(btns, mForwardText) )
            {
               panel.addControlText(btns, mForwardText);
            }
         }

         if ( mReverseBtn.mButtonVec[0] != -1 )
         {
            std::vector<int> btns(transformButtonVec(mReverseBtn.mButtonVec));
            if ( ! panel.hasControlText(btns, mReverseText) )
            {
               panel.addControlText(btns, mReverseText);
            }
         }

         if ( mRotateBtn.mButtonVec[0] != -1 )
         {
            std::vector<int> btns(transformButtonVec(mRotateBtn.mButtonVec));
            if ( ! panel.hasControlText(btns, mRotateText) )
            {
               panel.addControlText(btns, mRotateText);
            }
         }

         if ( mModeBtn.mButtonVec[0] != -1 )
         {
            std::vector<int> btns(transformButtonVec(mModeBtn.mButtonVec));
            if ( ! panel.hasControlText(btns, mModeText) )
            {
               panel.addControlText(btns, mModeText);
            }
         }

         if ( mResetBtn.mButtonVec[0] != -1 )
         {
            std::vector<int> btns(transformButtonVec(mResetBtn.mButtonVec));
            if ( ! panel.hasControlText(btns, mResetText) )
            {
               panel.addControlText(btns, mResetText);
            }
         }
      }
   }
}

void WandNavPlugin::updateNavState(ViewerPtr viewer,
                                   ViewPlatform& viewPlatform)
{
   vprASSERT(mWandInterface.get() != NULL && "No valid wand interface");

   // Update velocity
   if ( mForwardBtn.test(gadget::Digital::ON) )
   {
      mVelocity += mAcceleration;
   }
   else if ( mReverseBtn.test(gadget::Digital::ON) )
   {
      mVelocity -= mAcceleration;
   }
   else if ( mVelocity != 0.0f )
   {
      mVelocity = 0.0f;
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
   if ( mModeBtn.test(gadget::Digital::TOGGLE_ON) )
   {
      mNavMode = (mNavMode == WALK ? FLY : WALK);
      IOV_STATUS << "Mode: " << (mNavMode == WALK ? "Walk" : "Fly")
                << std::endl;
   }
   // If the accelerate button and the rotate button are pressed together,
   // then reset to the starting point translation and rotation.
   else if ( mResetBtn.test(gadget::Digital::ON) )
   {
      mNavState = RESET;
   }
   else if ( mRotateBtn.test(gadget::Digital::ON) )
   {
      mNavState = ROTATE;
   }
   else
   {
      mNavState = TRANSLATE;
   }
}

void WandNavPlugin::runNav(ViewerPtr viewer, ViewPlatform& viewPlatform)
{
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

      switch ( mNavState )
      {
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
            vprASSERT(false && "Bad value for mNavState");
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

struct StringToInt
{
   int operator()(const std::string& input)
   {
      return atoi(input.c_str());
   }
};

void WandNavPlugin::configButtons(jccl::ConfigElementPtr elt,
                                  const std::string& propName,
                                  WandNavPlugin::DigitalHolder& holder)
{
   holder.mWandIf = mWandInterface;

   std::string btn_str = elt->getProperty<std::string>(propName);

   std::vector<std::string> btn_strings;
   boost::trim(btn_str);
   boost::split(btn_strings, btn_str, boost::is_any_of(", "));
   holder.mButtonVec.resize(btn_strings.size());
   std::transform(btn_strings.begin(), btn_strings.end(),
                  holder.mButtonVec.begin(), StringToInt());
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

bool WandNavPlugin::DigitalHolder::test(const gadget::Digital::State testState)
{
   if ( mButtonVec.empty() )
   {
      return false;
   }
   else
   {
      mButtonState = testState;
      return std::accumulate(mButtonVec.begin(), mButtonVec.end(), true,
                             *this);
   }
}

bool WandNavPlugin::DigitalHolder::operator()(bool state, int btn)
{
   return state && mWandIf->getButton(btn)->getData() == mButtonState;
}

}
