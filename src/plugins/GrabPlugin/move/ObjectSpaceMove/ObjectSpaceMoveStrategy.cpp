// vrkit is (C) Copyright 2005-2011
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

#include <boost/bind.hpp>
#include <boost/assign/list_of.hpp>

#include <gmtl/Generate.h>
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

#include "ObjectSpaceMoveStrategy.h"


using namespace boost::assign;

static const vrkit::plugin::Info sInfo(
   "com.infiscape.move", "ObjectSpaceMoveStrategy",
   list_of(VRKIT_VERSION_MAJOR)(VRKIT_VERSION_MINOR)(VRKIT_VERSION_PATCH)
);
static vrkit::plugin::Creator<vrkit::move::Strategy> sPluginCreator(
   boost::bind(&vrkit::ObjectSpaceMoveStrategy::create, sInfo)
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

move::StrategyPtr ObjectSpaceMoveStrategy::init(ViewerPtr)
{
   return shared_from_this();
}

void ObjectSpaceMoveStrategy::
objectsGrabbed(ViewerPtr, const std::vector<SceneObjectPtr>& objs,
               const gmtl::Point3f&, const gmtl::Matrix44f& vp_M_wand)
{
   gmtl::invert(m_wand_M_vp, vp_M_wand);
   mRotation = gmtl::makeRot<gmtl::Quatf>(vp_M_wand);
   gmtl::invert(mRotation);

   std::for_each(objs.begin(), objs.end(),
                 boost::bind(&ObjectSpaceMoveStrategy::objectGrabbed, this,
                             _1, vp_M_wand));
}

void ObjectSpaceMoveStrategy::objectGrabbed(SceneObjectPtr obj,
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

   ObjectData& obj_data = mObjectDataMap[obj];
   obj_data.m_wand_M_pobj = wand_M_vp * vp_M_pobj;

   OSG::Matrix node_xform = obj->getPos();
   gmtl::set(obj_data.m_pobj_M_obj, node_xform);
   gmtl::invert(obj_data.m_obj_M_pobj, obj_data.m_pobj_M_obj);
}

void ObjectSpaceMoveStrategy::
objectsReleased(ViewerPtr, const std::vector<SceneObjectPtr>& objs)
{
   // This does not use std::for_each() because the cast needed to
   // disambiguate which std::map<...>::erase() overload to use would make
   // the code virtually unreadable.
   typedef std::vector<SceneObjectPtr>::const_iterator iter_type;
   for ( iter_type o = objs.begin(); o != objs.end(); ++o )
   {
      mObjectDataMap.erase(*o);
   }
}

gmtl::Matrix44f ObjectSpaceMoveStrategy::
computeMove(ViewerPtr viewer, SceneObjectPtr obj,
            const gmtl::Matrix44f& vp_M_wand,
            const gmtl::Matrix44f& curObjPos)
{
   gmtl::Matrix44f pobj_M_obj = curObjPos;
   gmtl::Matrix44f obj_M_pobj;
   gmtl::invert(obj_M_pobj, pobj_M_obj);

   OSG::Matrix world_xform;

   vprASSERT(obj->getRoot() != OSG::NullFC);

   // If we have no parent then we want to use the identity.
   if (obj->getRoot()->getParent() != OSG::NullFC)
   {
      obj->getRoot()->getParent()->getToWorld(world_xform);
   }

   gmtl::Matrix44f vp_M_pobj;
   gmtl::set(vp_M_pobj, world_xform);

   ObjectData& obj_data = mObjectDataMap[obj];

   gmtl::Matrix44f old_rot_only = m_wand_M_vp;
   //gmtl::setTrans(old_rot_only, gmtl::Vec3f(0.0f, 0.0f, 0.0f));

   gmtl::Matrix44f rot_only = vp_M_wand;
   //gmtl::setTrans(rot_only, gmtl::Vec3f(0.0f, 0.0f, 0.0f));

   //return m_wand_M_vp * vp_M_wand * curObjPos;
   //return rot_only * old_rot_only * curObjPos;
   //return vp_M_wand * m_wand_M_vp * curObjPos;

   //return rot_only * old_rot_only * curObjPos;
   //return obj_data.m_pobj_M_obj * rot_only * old_rot_only * obj_data.m_obj_M_pobj * curObjPos;
   gmtl::Quatf new_rot = gmtl::makeRot<gmtl::Quatf>(vp_M_wand);
   gmtl::Matrix44f orot = gmtl::makeRot<gmtl::Matrix44f>(mRotation);
   gmtl::Matrix44f nrot = gmtl::makeRot<gmtl::Matrix44f>(new_rot);

   std::cout << "vp_M_pobj\n" << vp_M_pobj << std::endl;
   std::cout << "m_pobj_M_obj\n" << obj_data.m_pobj_M_obj << std::endl;
   std::cout << "nrot\n" << nrot << std::endl;
   std::cout << "orot\n" << orot << std::endl;
   std::cout << "m_obj_M_pobj\n" << obj_data.m_obj_M_pobj << std::endl;
   std::cout << "curObjPos\n" << curObjPos << std::endl;
   //return obj_data.m_pobj_M_obj * nrot * orot * obj_data.m_obj_M_pobj * curObjPos;
   //return obj_data.m_pobj_M_obj * rot_only * old_rot_only * obj_data.m_obj_M_pobj * curObjPos;
   return pobj_M_obj * old_rot_only * rot_only * obj_M_pobj * curObjPos;
   //return nrot * orot * curObjPos;
}

}
