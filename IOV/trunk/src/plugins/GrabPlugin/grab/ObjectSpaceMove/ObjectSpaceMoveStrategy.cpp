// Copyright (C) Infiscape Corporation 2005-2006

#include <IOV/Config.h>

#include <gmtl/MatrixOps.h>
#include <gmtl/External/OpenSGConvert.h>

#include <IOV/Plugin/PluginConfig.h>
#include <IOV/PluginCreator.h>
#include <IOV/User.h>
#include <IOV/Viewer.h>
#include <IOV/WandInterface.h>

#include "ObjectSpaceMoveStrategy.h"


static inf::PluginCreator<inf::MoveStrategy> sPluginCreator(
   &inf::ObjectSpaceMoveStrategy::create, "Object Space Move Strategy Plug-in"
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

void ObjectSpaceMoveStrategy::init(inf::ViewerPtr viewer)
{
}

void ObjectSpaceMoveStrategy::objectGrabbed(inf::ViewerPtr viewer,
                                      OSG::TransformNodePtr obj,
                                      const gmtl::Point3f& intersectPoint,
                                      const gmtl::Matrix44f& vp_M_wand)
{
   gmtl::invert(m_wand_M_vp, vp_M_wand);
}

void ObjectSpaceMoveStrategy::objectReleased(inf::ViewerPtr viewer,
                                       OSG::TransformNodePtr obj)
{
   gmtl::identity(m_wand_M_vp);
}

gmtl::Matrix44f
ObjectSpaceMoveStrategy::computeMove(inf::ViewerPtr viewer,
                               OSG::TransformNodePtr obj,
                               const gmtl::Matrix44f& vp_M_wand,
                               gmtl::Matrix44f& curObjPos)
{
   return m_wand_M_vp * vp_M_wand * curObjPos;
}

}
