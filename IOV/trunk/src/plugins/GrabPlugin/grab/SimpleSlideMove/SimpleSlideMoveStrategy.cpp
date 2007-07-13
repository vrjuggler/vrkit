// Copyright (C) Infiscape Corporation 2005-2007

#include <IOV/Config.h>

#include <gmtl/MatrixOps.h>
#include <gmtl/External/OpenSGConvert.h>

#include <IOV/Plugin/PluginConfig.h>
#include <IOV/PluginCreator.h>
#include <IOV/SceneObject.h>
#include <IOV/Status.h>
#include <IOV/User.h>
#include <IOV/Viewer.h>
#include <IOV/WandInterface.h>
#include <IOV/Version.h>
#include <IOV/Plugin/Info.h>
#include <IOV/Util/Exceptions.h>

#include "SimpleSlideMoveStrategy.h"


static inf::PluginCreator<inf::MoveStrategy> sPluginCreator(
   &inf::SimpleSlideMoveStrategy::create
);

extern "C"
{

/** @name Plug-in Entry Points */
//@{
IOV_PLUGIN_API(inf::plugin::Info) getPluginInfo()
{
   std::vector<unsigned int> version(3);
   version[0] = IOV_VERSION_MAJOR;
   version[1] = IOV_VERSION_MINOR;
   version[2] = IOV_VERSION_PATCH;

   return inf::plugin::Info("com.infiscape.move", "SimpleSlideMoveStrategy",
                            version);
}

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

SimpleSlideMoveStrategy::SimpleSlideMoveStrategy()
   : inf::MoveStrategy()
   , mTransValue(0.0f)
   , mAnalogNum(0)
   , mForwardValue(1.0f)
   , mSlideEpsilon(0.1f)
   , mSlideMultiplier(0.2f)
{
   /* Do nothing. */ ;
}

inf::MoveStrategyPtr SimpleSlideMoveStrategy::init(inf::ViewerPtr viewer)
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

void SimpleSlideMoveStrategy::
objectsGrabbed(inf::ViewerPtr, const std::vector<SceneObjectPtr>&,
               const gmtl::Point3f&, const gmtl::Matrix44f&)
{
   /* Do nothing. */ ;
}

void SimpleSlideMoveStrategy::
objectsReleased(inf::ViewerPtr, const std::vector<SceneObjectPtr>&)
{
   mTransValue = 0.0f;
}

gmtl::Matrix44f
SimpleSlideMoveStrategy::computeMove(inf::ViewerPtr viewer,
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

      const float in_out_scale(mSlideMultiplier * viewer->getDrawScaleFactor());
      const float trans_val(gmtl::Math::pow(in_out_val, 5) * in_out_scale);
      mTransValue += trans_val;

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

      const gmtl::Matrix44f delta_trans_mat =
         gmtl::makeTrans<gmtl::Matrix44f>(gmtl::Vec3f(0.0f, 0.0f,
                                                      mTransValue));

      // Compute the new pobj_M_obj (the new local transformation for the
      // grabbed object) by factoring in the translation change in wand space.
      // NOTE: curObjPos = pobj_M_obj.
      return pobj_M_wand * delta_trans_mat * wand_M_pobj * curObjPos;
   }

   return curObjPos;
}

void SimpleSlideMoveStrategy::configure(jccl::ConfigElementPtr cfgElt)
{
   vprASSERT(cfgElt->getID() == getElementType());

   const unsigned int req_cfg_version(1);

   if ( cfgElt->getVersion() < req_cfg_version )
   {
      std::stringstream msg;
      msg << "Configuration of SimpleSlideMoveStrategy failed. Required "
          << "config element version is " << req_cfg_version
          << ", but element '" << cfgElt->getName() << "' is version "
          << cfgElt->getVersion();
      throw inf::PluginException(msg.str(), IOV_LOCATION);
   }

   const std::string analog_input_prop("analog_input");
   const std::string forward_value_prop("forward_slide_value");
   const std::string slide_epsilon_prop("slide_epsilon");
   const std::string slide_multiplier_prop("slide_multiplier");

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
      IOV_STATUS << "ERROR: Slide threshold must be non-negative!"
                 << std::endl;
   }

   const float slide_mult = cfgElt->getProperty<float>(slide_multiplier_prop);

   if ( slide_mult > 0.0f )
   {
      mSlideMultiplier = slide_mult;
   }
   else
   {
      IOV_STATUS << "ERROR: Slide acceleration multiplier must be greater "
                 << "than 0!" << std::endl;
   }
}

}
