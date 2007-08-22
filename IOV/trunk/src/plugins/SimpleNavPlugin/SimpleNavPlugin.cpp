// Copyright (C) Infiscape Corporation 2005-2007

#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#endif

#include <sstream>
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

#include <IOV/InterfaceTrader.h>
#include <IOV/Viewer.h>
#include <IOV/PluginCreator.h>
#include <IOV/WandInterface.h>
#include <IOV/User.h>
#include <IOV/Util/Exceptions.h>
#include <IOV/Status.h>
#include <IOV/StatusPanelData.h>
#include <IOV/Version.h>
#include <IOV/Plugin/Info.h>

#include "SimpleNavPlugin.h"


using namespace boost::assign;

static const inf::plugin::Info sInfo(
   "com.infiscape", "SimpleNavPlugin",
   list_of(IOV_VERSION_MAJOR)(IOV_VERSION_MINOR)(IOV_VERSION_PATCH)
);
static inf::PluginCreator<inf::Plugin> sPluginCreator(
   boost::bind(&inf::SimpleNavPlugin::create, sInfo)
);

extern "C"
{

/** @name Plug-in Entry Points */
//@{
IOV_PLUGIN_API(const inf::plugin::Info*) getPluginInfo()
{
   return &sInfo;
}

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

SimpleNavPlugin::SimpleNavPlugin(const inf::plugin::Info& info)
   : NavPlugin(info)
   , mCanNavigate(false)
   , mVelocity(0.0f)
   , mNavMode(WALK)
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
   const std::string for_btn_prop("forward_button_nums");
   const std::string rev_btn_prop("reverse_button_nums");
   const std::string rot_btn_prop("rotate_button_nums");
   const std::string mode_btn_prop("nav_mode_button_nums");
   const int unsigned req_cfg_version(2);

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
   mForBtn.configure(elt->getProperty<std::string>(for_btn_prop),
                     mWandInterface);
   mRevBtn.configure(elt->getProperty<std::string>(rev_btn_prop),
                     mWandInterface);
   mRotateBtn.configure(elt->getProperty<std::string>(rot_btn_prop),
                        mWandInterface);
   mModeBtn.configure(elt->getProperty<std::string>(mode_btn_prop),
                      mWandInterface);

   return shared_from_this();
}

void SimpleNavPlugin::focusChanged(inf::ViewerPtr viewer)
{
   inf::ScenePtr scene = viewer->getSceneObj();
   StatusPanelDataPtr status_panel_data =
      scene->getSceneData<StatusPanelData>();

   // We can only navigate when we have focus.
   mCanNavigate = isFocused();

   if ( ! mCanNavigate )
   {
      mVelocity = 0.0f;

      if ( mForBtn.isConfigured() )
      {
         status_panel_data->removeControlText(mForBtn.toString(),
                                              mForwardText);
      }

      if ( mRevBtn.isConfigured() )
      {
         status_panel_data->removeControlText(mRevBtn.toString(),
                                              mReverseText);
      }

      if ( mRotateBtn.isConfigured() )
      {
         status_panel_data->removeControlText(mRotateBtn.toString(),
                                              mRotateText);
      }

      if ( mModeBtn.isConfigured() )
      {
         status_panel_data->removeControlText(mModeBtn.toString(), mModeText);
      }
   }
   else
   {
      if ( mForBtn.isConfigured() )
      {
         const bool has =
            status_panel_data->hasControlText(mForBtn.toString(),
                                              mForwardText);

         if ( ! has )
         {
            status_panel_data->addControlText(mForBtn.toString(),
                                              mForwardText, 1);
         }
      }

      if ( mRevBtn.isConfigured() )
      {
         const bool has =
            status_panel_data->hasControlText(mRevBtn.toString(),
                                              mReverseText);

         if ( ! has )
         {
            status_panel_data->addControlText(mRevBtn.toString(),
                                              mReverseText, 1);
         }
      }

      if ( mRotateBtn.isConfigured() )
      {
         const bool has =
            status_panel_data->hasControlText(mRotateBtn.toString(),
                                              mRotateText);

         if ( ! has )
         {
            status_panel_data->addControlText(mRotateBtn.toString(),
                                              mRotateText, 1);
         }
      }

      if ( mModeBtn.isConfigured() )
      {
         const bool has =
            status_panel_data->hasControlText(mModeBtn.toString(), mModeText);

         if ( ! has )
         {
            status_panel_data->addControlText(mModeBtn.toString(), mModeText,
                                              1);
         }
      }
   }
}

void SimpleNavPlugin::updateNav(ViewerPtr viewer, ViewPlatform& viewPlatform)
{
   NavState nav_state(RESET);

   if ( isFocused() )
   {
      vprASSERT(mWandInterface.get() != NULL && "No valid wand interface");

      const float inc_vel(0.005f);
      const float max_vel(0.5f);

      // Update velocity
      if ( mForBtn() )
      {
         mVelocity += inc_vel;
      }
      else if ( mRevBtn() )
      {
         mVelocity -= inc_vel;
      }
      else if ( mVelocity > 0 )
      {
         mVelocity = 0.0f;
      }

      // Restrict range
      if ( mVelocity < -max_vel )
      {
         mVelocity = -max_vel;
      }
      if ( mVelocity > max_vel )
      {
         mVelocity = max_vel;
      }

      // Swap the navigation mode if the mode switching button was toggled on.
      if ( mModeBtn() )
      {
         mNavMode = (mNavMode == WALK ? FLY : WALK);
         IOV_STATUS << "Mode: " << (mNavMode == WALK ? "Walk" : "Fly")
                   << std::endl;
      }

      // If the accelerate button and the rotate button are pressed together,
      // then reset to the starting point translation and rotation.
      if ( mForBtn() && mRotateBtn() )
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

   if ( mCanNavigate )
   {
      gmtl::Matrix44f cur_pos = viewPlatform.getCurPos();

      const float scale_factor(viewer->getDrawScaleFactor());

      switch ( nav_state )
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
            vprASSERT(false && "Bad value in nav_state");
            break;
      }

      viewPlatform.setCurPos(cur_pos);
   }
}

}
