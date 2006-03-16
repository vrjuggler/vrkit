// Copyright (C) Infiscape Corporation 2005-2006

#include <IOV/Config.h>

#include <gmtl/MatrixOps.h>
#include <gmtl/External/OpenSGConvert.h>

#include <IOV/Plugin/PluginConfig.h>
#include <IOV/PluginCreator.h>

#include "BasicMoveStrategy.h"


static inf::PluginCreator<inf::MoveStrategy> sPluginCreator(
   &inf::BasicMoveStrategy::create, "Basic Move Strategy Plug-in"
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

void BasicMoveStrategy::init(inf::ViewerPtr viewer)
{
}

void BasicMoveStrategy::objectGrabbed(inf::ViewerPtr viewer,
                                      OSG::TransformNodePtr obj,
                                      const gmtl::Matrix44f& vp_M_wand)
{
   // m_xform_wand_M_obj_xform is the offset between the wand and the grabbed
   // object's center point:
   //
   //    m_xform_wand_M_obj_xform = xform_wand_M_vp * vp_M_obj * obj_xform
   gmtl::Matrix44f xform_wand_M_vp;
   gmtl::Matrix44f obj_xform;

   OSG::Matrix world_xform;
   obj.node()->getParent()->getToWorld(world_xform);
   gmtl::Matrix44f vp_M_obj;
   gmtl::set(vp_M_obj, world_xform);

   gmtl::invert(xform_wand_M_vp, vp_M_wand);
   gmtl::set(obj_xform, obj->getMatrix());

   m_xform_wand_M_obj_xform = xform_wand_M_vp * vp_M_obj * obj_xform;
}

void BasicMoveStrategy::objectReleased(inf::ViewerPtr viewer,
                                       OSG::TransformNodePtr obj)
{
   gmtl::identity(m_xform_wand_M_obj_xform);
}

gmtl::Matrix44f
BasicMoveStrategy::computeMove(inf::ViewerPtr viewer,
                               OSG::TransformNodePtr obj,
                               const gmtl::Matrix44f& vp_M_wand)
{
   // obj_M_vp is the inverse of the object in view platform space.
   OSG::Matrix world_xform;
   obj.node()->getParent()->getToWorld(world_xform);
   gmtl::Matrix44f obj_M_vp;
   gmtl::set(obj_M_vp, world_xform);
   gmtl::invert(obj_M_vp);

   // XXX: Move strategy
   const gmtl::Matrix44f obj_M_wand_xform = obj_M_vp * vp_M_wand;

   return obj_M_wand_xform * m_xform_wand_M_obj_xform;
   //gmtl::Matrix44f new_obj_xform = vp_M_wand_xform;
   //gmtl::Matrix44f new_obj_mat = new_obj_xform * m_obj_M_xform;
}

}
