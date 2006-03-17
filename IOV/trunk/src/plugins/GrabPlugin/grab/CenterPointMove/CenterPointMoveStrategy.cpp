// Copyright (C) Infiscape Corporation 2005-2006

#include <IOV/Config.h>

#include <gmtl/MatrixOps.h>
#include <gmtl/External/OpenSGConvert.h>

#include <IOV/Plugin/PluginConfig.h>
#include <IOV/PluginCreator.h>

#include "CenterPointMoveStrategy.h"


static inf::PluginCreator<inf::MoveStrategy> sPluginCreator(
   &inf::CenterPointMoveStrategy::create, "Center Point Move Strategy Plug-in"
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

void CenterPointMoveStrategy::init(inf::ViewerPtr)
{
   /* Do nothing. */ ;
}

void CenterPointMoveStrategy::objectGrabbed(inf::ViewerPtr,
                                            OSG::TransformNodePtr,
                                            const gmtl::Point3f&,
                                            const gmtl::Matrix44f&)
{
   /* Do nothing. */ ;
}

void CenterPointMoveStrategy::objectReleased(inf::ViewerPtr,
                                             OSG::TransformNodePtr)
{
   /* Do nothing. */ ;
}

gmtl::Matrix44f
CenterPointMoveStrategy::computeMove(inf::ViewerPtr,
                                     OSG::TransformNodePtr obj,
                                     const gmtl::Matrix44f& vp_M_wand,
                                     gmtl::Matrix44f&)
{
   // pobj_M_vp is the inverse of the object in view platform space.
   OSG::Matrix world_xform;
   obj.node()->getParent()->getToWorld(world_xform);
   gmtl::Matrix44f pobj_M_vp;
   gmtl::set(pobj_M_vp, world_xform);
   gmtl::invert(pobj_M_vp);

   return pobj_M_vp * vp_M_wand;
}

}
