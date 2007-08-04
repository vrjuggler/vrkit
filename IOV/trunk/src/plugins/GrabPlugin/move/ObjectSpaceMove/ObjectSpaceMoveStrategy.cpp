// Copyright (C) Infiscape Corporation 2005-2007

#include <IOV/Config.h>

#include <boost/bind.hpp>
#include <boost/assign/list_of.hpp>

#include <gmtl/Generate.h>
#include <gmtl/MatrixOps.h>
#include <gmtl/External/OpenSGConvert.h>

#include <IOV/Plugin/PluginConfig.h>
#include <IOV/PluginCreator.h>
#include <IOV/User.h>
#include <IOV/Viewer.h>
#include <IOV/WandInterface.h>
#include <IOV/SceneObject.h>
#include <IOV/Version.h>
#include <IOV/Plugin/Info.h>

#include "ObjectSpaceMoveStrategy.h"


using namespace boost::assign;

static const inf::plugin::Info sInfo(
   "com.infiscape.move", "ObjectSpaceMoveStrategy",
   list_of(IOV_VERSION_MAJOR)(IOV_VERSION_MINOR)(IOV_VERSION_PATCH)
);
static inf::PluginCreator<inf::MoveStrategy> sPluginCreator(
   boost::bind(&inf::ObjectSpaceMoveStrategy::create, sInfo)
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

inf::MoveStrategyPtr ObjectSpaceMoveStrategy::init(inf::ViewerPtr)
{
   return shared_from_this();
}

void ObjectSpaceMoveStrategy::
objectsGrabbed(inf::ViewerPtr, const std::vector<SceneObjectPtr>& objs,
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
objectsReleased(inf::ViewerPtr, const std::vector<SceneObjectPtr>& objs)
{
   // This does not use std::for_each() because the cast needed to
   // disambiguate which std::map<...>::erase() overload to use would make
   // the code virtually unreadable.
   std::vector<SceneObjectPtr>::const_iterator o;
   for ( o = objs.begin(); o != objs.end(); ++o )
   {
      mObjectDataMap.erase(*o);
   }
}

gmtl::Matrix44f ObjectSpaceMoveStrategy::
computeMove(inf::ViewerPtr viewer, SceneObjectPtr obj,
            const gmtl::Matrix44f& vp_M_wand, const gmtl::Matrix44f& curObjPos)
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
