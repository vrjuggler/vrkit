// Copyright (C) Infiscape Corporation 2005-2006

#include <IOV/Config.h>

#include <gmtl/MatrixOps.h>
#include <gmtl/External/OpenSGConvert.h>
#include <gmtl/Output.h>

#include <IOV/Plugin/PluginConfig.h>
#include <IOV/PluginCreator.h>
#include <IOV/SceneObject.h>
#include <IOV/Status.h>
#include <IOV/User.h>
#include <IOV/Viewer.h>
#include <IOV/WandInterface.h>
#include <IOV/Util/Exceptions.h>

#include "SlideMoveStrategy.h"


static inf::PluginCreator<inf::MoveStrategy> sPluginCreator(
   &inf::SlideMoveStrategy::create, "Slide Move Strategy Plug-in"
);

extern "C"
{

/** @name Plug-in Entry Points */
//@{
IOV_PLUGIN_API(void) getPluginInterfaceVersion(vpr::Uint32& majorVer,
                                               vpr::Uint32& minorVer)
{
   majorVer = INF_MOVE_STRATEGY_PLUGIN_API_MAJOR;
   minorVer = INF_MOVE_STRATEGY_PLUGIN_API_MINOR;
}

IOV_PLUGIN_API(inf::PluginCreatorBase*) getMoveStrategyCreator()
{
   return &sPluginCreator;
}
//@}

}

namespace inf
{

SlideMoveStrategy::SlideMoveStrategy()
   : inf::MoveStrategy()
   , mTransValue(0.0f)
   , mAnalogNum(0)
   , mForwardValue(1.0f)
   , mSlideEpsilon(0.1f)
{
   /* Do nothing. */ ;
}

inf::MoveStrategyPtr SlideMoveStrategy::init(inf::ViewerPtr viewer)
{
   jccl::ConfigElementPtr cfg_elt =
      viewer->getConfiguration().getConfigElement(getElementType());

   if ( cfg_elt )
   {
      try
      {
         configure(cfg_elt);
      }
      catch (inf::Exception& ex)
      {
         std::cerr << ex.what() << std::endl;
      }
   }

   vprASSERT(mForwardValue == 0.0f || mForwardValue == 1.0f &&
             "Invalid setting for mForwardValue");

   return shared_from_this();
}

void SlideMoveStrategy::objectsGrabbed(inf::ViewerPtr,
                                       const std::vector<SceneObjectPtr>&,
                                       const gmtl::Point3f& intersectPoint,
                                       const gmtl::Matrix44f&)
{
   mIntersectPoint = intersectPoint;
}

void SlideMoveStrategy::objectsReleased(inf::ViewerPtr,
                                        const std::vector<SceneObjectPtr>&)
{
   mTransValue = 0.0f;
}

gmtl::Matrix44f
SlideMoveStrategy::computeMove(inf::ViewerPtr viewer,
                               SceneObjectPtr obj,
                               const gmtl::Matrix44f& vp_M_wand,
                               const gmtl::Matrix44f& curObjPos)
{
   WandInterfacePtr wand_if =
      viewer->getUser()->getInterfaceTrader().getWandInterface();
   gadget::AnalogInterface& analog_dev = wand_if->getAnalog(mAnalogNum);

   if ( ! analog_dev->isStupefied() )
   {
      const float analog_value(analog_dev->getData());
      //std::cout << "Analog Value: " << analog_value << std::endl;

      // Rescale [0,1] to [-1,1]
      float in_out_val = analog_value * 2.0 - 1.0f;

      if ( gmtl::Math::abs(in_out_val) < mSlideEpsilon )
      {
         in_out_val = 0.0f;
      }

      // The above code treats the forward value as 1.0. If the forward value
      // is 0.0, then swap the sliding direction.
      if ( mForwardValue == 0.0f )
      {
         in_out_val = -in_out_val;
      }

      // If we don't have an analog configured, then set in-out to 0.
      if ( -1 == mAnalogNum )
      {
         in_out_val = 0.0f;
      }

      //std::cout << "In out: " << in_out_val << std::endl;

      const float in_out_scale(0.20f);
      const float trans_val(in_out_val * in_out_scale);
      mTransValue += trans_val;

      // If the object is at the wand, then don't allow it to move any
      // further back.
      if ( mTransValue >= 1.0 )
      {
         mTransValue = 1.0;
      }

      OSG::Matrix world_xform;

      vprASSERT(obj->getRoot() != OSG::NullFC);

      // If we have no parent then we want to use the identity.
      if ( obj->getRoot()->getParent() != OSG::NullFC )
      {
         // Gets vp_M_pobj.
         obj->getRoot()->getParent()->getToWorld(world_xform);
      }

      // pobj_M_vp is the inverse of the coordinate frame for obj in view
      // platform space.
      gmtl::Matrix44f pobj_M_vp;
      gmtl::set(pobj_M_vp, world_xform);
      gmtl::invert(pobj_M_vp);

      const gmtl::Matrix44f pobj_M_wand = pobj_M_vp * vp_M_wand;
      gmtl::Matrix44f wand_M_pobj;
      gmtl::invert(wand_M_pobj, pobj_M_wand);

      const gmtl::Matrix44f wand_M_obj(wand_M_pobj * curObjPos);

      gmtl::Vec3f obj_dir = gmtl::makeTrans<gmtl::Vec3f>(wand_M_obj);

      if ( mSlideTarget == ISECT_POINT )
      {
         obj_dir += mIntersectPoint;
      }

      // XXX: This was removed to allow objects further away to get
      //      closer faster. If this is normalized again then we will
      //      need to change the mTransVal >= check to take the length
      //      of obj_dir into account.
      //gmtl::normalize(obj_dir);

      // Accumulate translation matrix.
      const gmtl::Matrix44f delta_trans_mat =
         gmtl::makeTrans<gmtl::Matrix44f>(obj_dir * -mTransValue);

      // Compute the new pobj_M_obj' (the new local transformation for the
      // grabbed object) by factoring in the translation change in wand space.
      // NOTE: curObjPos == pobj_M_obj.
      return pobj_M_wand * delta_trans_mat * wand_M_pobj * curObjPos;
   }

   return curObjPos;
}

void SlideMoveStrategy::configure(jccl::ConfigElementPtr cfgElt)
{
   vprASSERT(cfgElt->getID() == getElementType());

   const unsigned int req_cfg_version(2);

   if ( cfgElt->getVersion() < req_cfg_version )
   {
      std::stringstream msg;
      msg << "Configuration of SlideMoveStrategy failed.  Required config "
          << "element version is " << req_cfg_version << ", but element '"
          << cfgElt->getName() << "' is version " << cfgElt->getVersion();
      throw inf::PluginException(msg.str(), IOV_LOCATION);
   }

   const std::string slide_target_prop("slide_target");
   const std::string analog_input_prop("analog_input");
   const std::string forward_value_prop("forward_slide_value");
   const std::string slide_epsilon_prop("slide_epsilon");

   const unsigned int slide_target =
      cfgElt->getProperty<unsigned int>(slide_target_prop);

   if ( slide_target == 0 || slide_target == 1 )
   {
      mSlideTarget = static_cast<SlideTarget>(slide_target);
   }
   else
   {
      IOV_STATUS << "ERROR: Invalid slide target identifier " << slide_target
                 << std::endl;
   }

   const int analog_num = cfgElt->getProperty<int>(analog_input_prop);

   // -1 disables sliding. Otherwise, the analog number must be in the range
   // 0 <= analog_num < 4.
   if ( analog_num == -1 || 0 <= analog_num && analog_num < 4 )
   {
      mAnalogNum = analog_num;
   }
   else
   {
      IOV_STATUS << "ERROR: Analog input index (" << analog_num << ") given!\n"
                 << "This must be -1 (to disable) or in the range [0, 4)."
                 << std::endl;
   }

   const float fwd_val = cfgElt->getProperty<float>(forward_value_prop);

   if ( fwd_val == 0.0f || fwd_val == 1.0f )
   {
      mForwardValue = fwd_val;
   }
   else
   {
      IOV_STATUS << "ERROR: Invalid forward sliding value (" << fwd_val
                 << ") given!\n"
                 << "This must be either 0.0 or 1.0" << std::endl;
   }

   const float eps = cfgElt->getProperty<float>(slide_epsilon_prop);

   if ( eps >= 0.0f )
   {
      mSlideEpsilon = eps;
   }
   else
   {
      IOV_STATUS << "ERROR: Slide epsilon must be non-negative!" << std::endl;
   }
}

}
