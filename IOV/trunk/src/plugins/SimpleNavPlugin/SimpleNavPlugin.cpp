// Copyright (C) Infiscape Corporation 2005-2006

#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#endif

#include <sstream>

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

#include "SimpleNavPlugin.h"


static inf::PluginCreator<inf::Plugin> sPluginCreator(
   &inf::SimpleNavPlugin::create, "Simple Navigator Plug-in"
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

SimpleNavPlugin::SimpleNavPlugin()
   : mCanNavigate(false)
   , mNavState(RESET)
   , mVelocity(0.0f)
   , mNavMode(WALK)
   , mForBtn(-1)
   , mRevBtn(-1)
   , mRotateBtn(-1)
   , mModeBtn(-1)
   , mForwardText("Forward")
   , mReverseText("Reverse")
   , mRotateText("Rotate")
   , mModeText("Toggle Walk/Fly")
{
   mCanNavigate = isFocused();
}

PluginPtr SimpleNavPlugin::init(ViewerPtr viewer)
{
   const std::string plugin_path_prop("plugin_path");
   const std::string plugin_prop("plugin");
   const std::string for_btn_prop("forward_button_num");
   const std::string rev_btn_prop("reverse_button_num");
   const std::string rot_btn_prop("rotate_button_num");
   const std::string mode_btn_prop("nav_mode_button_num");
   const int unsigned req_cfg_version(1);

   InterfaceTrader& if_trader = viewer->getUser()->getInterfaceTrader();
   mWandInterface = if_trader.getWandInterface();

   // -- Configure -- //
   std::string elt_type_name = getElementType();
   jccl::ConfigElementPtr elt = viewer->getConfiguration().getConfigElement(elt_type_name);

   if(!elt)
   {
      throw PluginException("SimpleNavPlugin could not find its configuration.", IOV_LOCATION);
   }
   vprASSERT(elt->getID() == getElementType());

   // Check for correct version of plugin configuration
   if(elt->getVersion() < req_cfg_version)
   {
      std::stringstream msg;
      msg << "Configuration of SimpleNavPlugin failed.  Required config "
          << "element version is " << req_cfg_version << ", but element '"
          << elt->getName() << "' is version " << elt->getVersion();
      throw PluginException(msg.str(), IOV_LOCATION);
   }

   // Get the button for navigation
   mForBtn = elt->getProperty<int>(for_btn_prop);
   mRevBtn = elt->getProperty<int>(rev_btn_prop);
   mRotateBtn = elt->getProperty<int>(rot_btn_prop);
   mModeBtn = elt->getProperty<int>(mode_btn_prop);

   return shared_from_this();
}

void SimpleNavPlugin::focusChanged(inf::ViewerPtr viewer)
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

         if ( mForBtn != -1 )
         {
            panel.removeControlText(mForBtn + 1, mForwardText);
         }

         if ( mRevBtn != -1 )
         {
            panel.removeControlText(mRevBtn + 1, mReverseText);
         }

         if ( mRotateBtn != -1 )
         {
            panel.removeControlText(mRotateBtn + 1, mRotateText);
         }

         if ( mModeBtn != -1 )
         {
            panel.removeControlText(mModeBtn + 1, mModeText);
         }
      }
   }
   else
   {
      if ( status_panel_data->mStatusPanelPlugin )
      {
         inf::StatusPanel& panel =
            status_panel_data->mStatusPanelPlugin->getPanel();

         if ( mForBtn != -1 )
         {
            if ( ! panel.hasControlText(mForBtn + 1, mForwardText) )
            {
               panel.addControlText(mForBtn + 1, mForwardText);
            }
         }

         if ( mRevBtn != -1 )
         {
            if ( ! panel.hasControlText(mRevBtn + 1, mReverseText) )
            {
               panel.addControlText(mRevBtn + 1, mReverseText);
            }
         }

         if ( mRotateBtn != -1 )
         {
            if ( ! panel.hasControlText(mRotateBtn + 1, mRotateText) )
            {
               panel.addControlText(mRotateBtn + 1, mRotateText);
            }
         }

         if ( mModeBtn != -1 )
         {
            if ( ! panel.hasControlText(mModeBtn + 1, mModeText) )
            {
               panel.addControlText(mModeBtn + 1, mModeText);
            }
         }
      }
   }
}

void SimpleNavPlugin::updateNavState(ViewerPtr viewer,
                                     ViewPlatform& viewPlatform)
{
   vprASSERT(mWandInterface.get() != NULL && "No valid wand interface");

   const float inc_vel(0.005f);
   const float max_vel(0.5f);

   gadget::DigitalInterface& accel_button =
      mWandInterface->getButton(mForBtn);
   gadget::DigitalInterface& deccel_button =
      mWandInterface->getButton(mRevBtn);
   gadget::DigitalInterface& rotate_button =
      mWandInterface->getButton(mRotateBtn);
   gadget::DigitalInterface& mode_button =
      mWandInterface->getButton(mModeBtn);

   // Update velocity
   if ( accel_button->getData() == gadget::Digital::ON )
   {
      mVelocity += inc_vel;
   }
   else if ( deccel_button->getData() == gadget::Digital::ON )
   {
      mVelocity -= inc_vel;
   }
   else if(mVelocity > 0)
   {
      mVelocity = 0.0f;
   }

   // Restrict range
   if(mVelocity < -max_vel)
   {
      mVelocity = -max_vel;
   }
   if(mVelocity > max_vel)
   {
      mVelocity = max_vel;
   }

   // Swap the navigation mode if the mode switching button was toggled on.
   if ( mode_button->getData() == gadget::Digital::TOGGLE_ON )
   {
      mNavMode = (mNavMode == WALK ? FLY : WALK);
      IOV_STATUS << "Mode: " << (mNavMode == WALK ? "Walk" : "Fly")
                << std::endl;
   }

   // If the accelerate button and the rotate button are pressed together,
   // then reset to the starting point translation and rotation.
   if ( accel_button->getData() && rotate_button->getData() )
   {
      mNavState = RESET;
   }
   else if ( rotate_button->getData() == gadget::Digital::ON )
   {
      mNavState = ROTATE;
   }
   else
   {
      mNavState = TRANSLATE;
   }
}

void SimpleNavPlugin::runNav(ViewerPtr viewer, ViewPlatform& viewPlatform)
{
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
               gmtl::Matrix44f rot_mat(
                  mWandInterface->getWandPos()->getData(scale_factor)
               );
               gmtl::setTrans(rot_mat, gmtl::Vec3f(0.0f, 0.0f, 0.0f));

               if ( gmtl::MAT_IDENTITY44F != rot_mat )
               {
                  float y_rot = gmtl::makeYRot(rot_mat);
                  gmtl::Quatf goal_quat(0.0f, 1.0f, 0.0f, y_rot);

                  gmtl::Quatf source_quat;
                  gmtl::Quatf slerp_quat;
                  // XXX: This needs to be time-based rotation.  This value of
                  // 0.005f is to compensate for a high frame rate with a
                  // simple test model.
                  gmtl::slerp(slerp_quat, 0.005f, source_quat, goal_quat);

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
               gmtl::Vec3f z_dir = gmtl::Vec3f(0.0f, 0.0f, -mVelocity);
               gmtl::Vec3f trans = wand_mat * z_dir;

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
            vprASSERT(false && "Bad value in mNavState");
            break;
      }

      viewPlatform.setCurPos(cur_pos);
   }
}

}
