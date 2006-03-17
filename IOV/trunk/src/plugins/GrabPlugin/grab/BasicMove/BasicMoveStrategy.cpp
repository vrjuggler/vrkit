// Copyright (C) Infiscape Corporation 2005-2006

#include <IOV/Config.h>

#include <gmtl/MatrixOps.h>
#include <gmtl/External/OpenSGConvert.h>

#include <IOV/Plugin/PluginConfig.h>
#include <IOV/PluginCreator.h>
#include <IOV/User.h>
#include <IOV/Viewer.h>
#include <IOV/WandInterface.h>

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
   // m_wand_M_pobj is the offset between the wand and the grabbed
   // object's center point:
   //
   //    m_wand_M_pobj = wand_M_vp * vp_M_pobj
   gmtl::Matrix44f wand_M_vp;

   OSG::Matrix world_xform;
   obj.node()->getParent()->getToWorld(world_xform);
   gmtl::Matrix44f vp_M_pobj;
   gmtl::set(vp_M_pobj, world_xform);

   gmtl::invert(wand_M_vp, vp_M_wand);

   m_wand_M_pobj = wand_M_vp * vp_M_pobj;
}

void BasicMoveStrategy::objectReleased(inf::ViewerPtr viewer,
                                       OSG::TransformNodePtr obj)
{
   gmtl::identity(m_wand_M_pobj);
}

gmtl::Matrix44f
BasicMoveStrategy::computeMove(inf::ViewerPtr viewer,
                               OSG::TransformNodePtr obj,
                               const gmtl::Matrix44f& vp_M_wand,
                               gmtl::Matrix44f& curObjMat)
{
   // pobj_M_vp is the inverse of the object in view platform space.
   OSG::Matrix world_xform;
   obj.node()->getParent()->getToWorld(world_xform);
   gmtl::Matrix44f pobj_M_vp;
   gmtl::set(pobj_M_vp, world_xform);
   gmtl::invert(pobj_M_vp);

   const gmtl::Matrix44f pobj_M_wand = pobj_M_vp * vp_M_wand;

   return pobj_M_wand * m_wand_M_pobj * curObjMat;
}

}
