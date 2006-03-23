// Copyright (C) Infiscape Corporation 2005-2006

#include <IOV/Config.h>

#include <gmtl/Generate.h>
#include <gmtl/MatrixOps.h>
#include <gmtl/External/OpenSGConvert.h>

#include <IOV/Plugin/PluginConfig.h>
#include <IOV/PluginCreator.h>
#include <IOV/User.h>
#include <IOV/Viewer.h>
#include <IOV/WandInterface.h>
#include <IOV/SceneObject.h>

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
                                      SceneObjectPtr obj,
                                      const gmtl::Point3f& intersectPoint,
                                      const gmtl::Matrix44f& vp_M_wand)
{
   gmtl::invert(m_wand_M_vp, vp_M_wand);
   mRotation = gmtl::makeRot<gmtl::Quatf>(vp_M_wand);
   gmtl::invert(mRotation);


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

   m_wand_M_pobj = wand_M_vp * vp_M_pobj;


   OSG::Matrix node_xform = obj->getPos();
   gmtl::set(m_pobj_M_obj, node_xform);
   gmtl::invert(m_obj_M_pobj, m_pobj_M_obj);
}

void ObjectSpaceMoveStrategy::objectReleased(inf::ViewerPtr viewer,
                                       SceneObjectPtr obj)
{
   gmtl::identity(m_wand_M_vp);
   gmtl::identity(m_wand_M_pobj);
}

gmtl::Matrix44f
ObjectSpaceMoveStrategy::computeMove(inf::ViewerPtr viewer,
                               SceneObjectPtr obj,
                               const gmtl::Matrix44f& vp_M_wand,
                               gmtl::Matrix44f& curObjPos)
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

   gmtl::Matrix44f old_rot_only = m_wand_M_vp;
   //gmtl::setTrans(old_rot_only, gmtl::Vec3f(0.0f, 0.0f, 0.0f));

   gmtl::Matrix44f rot_only = vp_M_wand;
   //gmtl::setTrans(rot_only, gmtl::Vec3f(0.0f, 0.0f, 0.0f));
   
   //return m_wand_M_vp * vp_M_wand * curObjPos;
   //return rot_only * old_rot_only * curObjPos;
   //return vp_M_wand * m_wand_M_vp * curObjPos;

   //return rot_only * old_rot_only * curObjPos;
   //return m_pobj_M_obj * rot_only * old_rot_only * m_obj_M_pobj * curObjPos;
   gmtl::Quatf new_rot = gmtl::makeRot<gmtl::Quatf>(vp_M_wand);
   gmtl::Matrix44f orot = gmtl::makeRot<gmtl::Matrix44f>(mRotation);
   gmtl::Matrix44f nrot = gmtl::makeRot<gmtl::Matrix44f>(new_rot);
   
   std::cout << "vp_M_pobj\n" << vp_M_pobj << std::endl;
   std::cout << "m_pobj_M_obj\n" << m_pobj_M_obj << std::endl;
   std::cout << "nrot\n" << nrot << std::endl;
   std::cout << "orot\n" << orot << std::endl;
   std::cout << "m_obj_M_pobj\n" << m_obj_M_pobj << std::endl;
   std::cout << "curObjPos\n" << curObjPos << std::endl;
   //return m_pobj_M_obj * nrot * orot * m_obj_M_pobj * curObjPos;
   //return m_pobj_M_obj * rot_only * old_rot_only * m_obj_M_pobj * curObjPos;
   return pobj_M_obj * old_rot_only * rot_only * obj_M_pobj * curObjPos;
   //return nrot * orot * curObjPos;

}

}
