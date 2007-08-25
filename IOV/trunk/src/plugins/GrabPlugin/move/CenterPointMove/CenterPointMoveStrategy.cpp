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

#include <IOV/Config.h>

#include <boost/bind.hpp>
#include <boost/assign/list_of.hpp>

#include <gmtl/MatrixOps.h>
#include <gmtl/External/OpenSGConvert.h>

#include <vpr/vpr.h>
#include <vpr/Util/Assert.h>

#include <IOV/Plugin/PluginConfig.h>
#include <IOV/PluginCreator.h>
#include <IOV/SceneObject.h>
#include <IOV/Version.h>
#include <IOV/Plugin/Info.h>

#include "CenterPointMoveStrategy.h"


using namespace boost::assign;

static const inf::plugin::Info sInfo(
   "com.infiscape.move", "CenterPointMoveStrategy",
   list_of(IOV_VERSION_MAJOR)(IOV_VERSION_MINOR)(IOV_VERSION_PATCH)
);
static inf::PluginCreator<inf::MoveStrategy> sPluginCreator(
   boost::bind(&inf::CenterPointMoveStrategy::create, sInfo)
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

inf::MoveStrategyPtr CenterPointMoveStrategy::init(inf::ViewerPtr)
{
   return shared_from_this();
}

void CenterPointMoveStrategy::
objectsGrabbed(inf::ViewerPtr, const std::vector<SceneObjectPtr>&,
               const gmtl::Point3f&, const gmtl::Matrix44f&)
{
   /* Do nothing. */ ;
}

void CenterPointMoveStrategy::
objectsReleased(inf::ViewerPtr, const std::vector<SceneObjectPtr>&)
{
   /* Do nothing. */ ;
}

gmtl::Matrix44f CenterPointMoveStrategy::
computeMove(inf::ViewerPtr, SceneObjectPtr obj,
            const gmtl::Matrix44f& vp_M_wand, const gmtl::Matrix44f&)
{
   // pobj_M_vp is the inverse of the object in view platform space.
   OSG::Matrix world_xform;

   vprASSERT(obj->getRoot() != OSG::NullFC);

   // If we have no parent then we want to use the identity.
   if (obj->getRoot()->getParent() != OSG::NullFC)
   {
      obj->getRoot()->getParent()->getToWorld(world_xform);
   }

   gmtl::Matrix44f pobj_M_vp;
   gmtl::set(pobj_M_vp, world_xform);
   gmtl::invert(pobj_M_vp);

   return pobj_M_vp * vp_M_wand;
}

}
