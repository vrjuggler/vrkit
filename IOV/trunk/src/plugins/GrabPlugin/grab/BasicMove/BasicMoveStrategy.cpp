// Copyright (C) Infiscape Corporation 2005-2007

#include <IOV/Config.h>

#include <algorithm>
#include <boost/bind.hpp>

#include <gmtl/MatrixOps.h>
#include <gmtl/External/OpenSGConvert.h>

#include <IOV/Plugin/PluginConfig.h>
#include <IOV/PluginCreator.h>
#include <IOV/User.h>
#include <IOV/Viewer.h>
#include <IOV/WandInterface.h>
#include <IOV/SceneObject.h>

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

inf::MoveStrategyPtr BasicMoveStrategy::init(inf::ViewerPtr)
{
   return shared_from_this();
}

void BasicMoveStrategy::objectsGrabbed(inf::ViewerPtr,
                                       const std::vector<SceneObjectPtr>& objs,
                                       const gmtl::Point3f&,
                                       const gmtl::Matrix44f& vp_M_wand)
{
   std::for_each(objs.begin(), objs.end(),
                 boost::bind(&BasicMoveStrategy::objectGrabbed, this, _1,
                             vp_M_wand));
}

void BasicMoveStrategy::
objectsReleased(inf::ViewerPtr, const std::vector<SceneObjectPtr>& objs)
{
   // This does not use std::for_each() because the cast needed to
   // disambiguate which std::map<...>::erase() overload to use would make
   // the code virtually unreadable.
   std::vector<SceneObjectPtr>::const_iterator o;
   for ( o = objs.begin(); o != objs.end(); ++o )
   {
      m_wand_M_pobj_map.erase(*o);
   }
}

gmtl::Matrix44f
BasicMoveStrategy::computeMove(inf::ViewerPtr, SceneObjectPtr obj,
                               const gmtl::Matrix44f& vp_M_wand,
                               const gmtl::Matrix44f& curObjMat)
{
   // pobj_M_vp is the inverse of the object in view platform space.
   OSG::Matrix world_xform;
   vprASSERT(obj->getRoot() != OSG::NullFC);
   vprASSERT(m_wand_M_pobj_map.count(obj) != 0);

   // If we have no parent then we want to use the identity.
   if (obj->getRoot()->getParent() != OSG::NullFC)
   {
      obj->getRoot()->getParent()->getToWorld(world_xform);
   }

   gmtl::Matrix44f pobj_M_vp;
   gmtl::set(pobj_M_vp, world_xform);
   gmtl::invert(pobj_M_vp);

   const gmtl::Matrix44f pobj_M_wand = pobj_M_vp * vp_M_wand;

   return pobj_M_wand * m_wand_M_pobj_map[obj] * curObjMat;
}

void BasicMoveStrategy::objectGrabbed(SceneObjectPtr obj,
                                      const gmtl::Matrix44f& vp_M_wand)
{
   // m_wand_M_pobj is the offset between the wand and the grabbed
   // object's center point:
   //
   //    m_wand_M_pobj = wand_M_vp * vp_M_pobj
   gmtl::Matrix44f wand_M_vp;

   OSG::Matrix world_xform;

   vprASSERT(obj->getRoot() != OSG::NullFC);

   // If we have no parent then we want to use the identity.
   if (obj->getRoot()->getParent() != OSG::NullFC)
   {
      obj->getRoot()->getParent()->getToWorld(world_xform);
   }

   gmtl::Matrix44f vp_M_pobj;
   gmtl::set(vp_M_pobj, world_xform);

   gmtl::invert(wand_M_vp, vp_M_wand);

   m_wand_M_pobj_map[obj] = wand_M_vp * vp_M_pobj;
}

}
