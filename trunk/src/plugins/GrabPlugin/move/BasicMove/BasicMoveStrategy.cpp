// vrkit is (C) Copyright 2005-2007
//    by Allen Bierbaum, Aron Bierbuam, Patrick Hartling, and Daniel Shipton
//
// This file is part of vrkit.
//
// vrkit is free software; you can redistribute it and/or modify it under the
// terms of the GNU Lesser General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// vrkit is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
// more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <vrkit/Config.h>

#include <algorithm>
#include <boost/bind.hpp>
#include <boost/assign/list_of.hpp>

#include <gmtl/MatrixOps.h>
#include <gmtl/External/OpenSGConvert.h>

#include <vrkit/plugin/Config.h>
#include <vrkit/User.h>
#include <vrkit/Viewer.h>
#include <vrkit/WandInterface.h>
#include <vrkit/SceneObject.h>
#include <vrkit/Version.h>
#include <vrkit/plugin/Creator.h>
#include <vrkit/plugin/Info.h>

#include "BasicMoveStrategy.h"


using namespace boost::assign;

static const vrkit::plugin::Info sInfo(
   "com.infiscape.move", "BasicMoveStrategy",
   list_of(VRKIT_VERSION_MAJOR)(VRKIT_VERSION_MINOR)(VRKIT_VERSION_PATCH)
);
static vrkit::plugin::Creator<vrkit::move::Strategy> sPluginCreator(
   boost::bind(&vrkit::BasicMoveStrategy::create, sInfo)
);

extern "C"
{

/** @name Plug-in Entry Points */
//@{
VRKIT_PLUGIN_API(const vrkit::plugin::Info*) getPluginInfo()
{
   return &sInfo;
}

VRKIT_PLUGIN_API(void) getPluginInterfaceVersion(vpr::Uint32& majorVer,
                                                 vpr::Uint32& minorVer)
{
   majorVer = VRKIT_MOVE_STRATEGY_PLUGIN_API_MAJOR;
   minorVer = VRKIT_MOVE_STRATEGY_PLUGIN_API_MINOR;
}

VRKIT_PLUGIN_API(vrkit::plugin::CreatorBase*) getMoveStrategyCreator()
{
   return &sPluginCreator;
}
//@}

}

namespace vrkit
{

move::StrategyPtr BasicMoveStrategy::init(ViewerPtr)
{
   return shared_from_this();
}

void BasicMoveStrategy::objectsGrabbed(ViewerPtr,
                                       const std::vector<SceneObjectPtr>& objs,
                                       const gmtl::Point3f&,
                                       const gmtl::Matrix44f& vp_M_wand)
{
   std::for_each(objs.begin(), objs.end(),
                 boost::bind(&BasicMoveStrategy::objectGrabbed, this, _1,
                             vp_M_wand));
}

void
BasicMoveStrategy::objectsReleased(ViewerPtr,
                                   const std::vector<SceneObjectPtr>& objs)
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
BasicMoveStrategy::computeMove(ViewerPtr, SceneObjectPtr obj,
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
