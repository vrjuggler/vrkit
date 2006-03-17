// Copyright (C) Infiscape Corporation 2005-2006

#include "PointIntersectionStrategy.h"
#include <gmtl/Matrix.h>
#include <gmtl/External/OpenSGConvert.h>
#include <IOV/GrabData.h>
#include <IOV/InterfaceTrader.h>
#include <IOV/Plugin/PluginConfig.h>
#include <IOV/PluginCreator.h>
#include <IOV/User.h>
#include <IOV/Viewer.h>
#include <IOV/WandInterface.h>

static inf::PluginCreator<inf::IntersectionStrategy> sPluginCreator(
   &inf::PointIntersectionStrategy::create, "Point Intersection Strategy Plug-in"
);

extern "C"
{

/** @name Plug-in Entry Points */
//@{
IOV_PLUGIN_API(void) getPluginInterfaceVersion(vpr::Uint32& majorVer,
                                               vpr::Uint32& minorVer)
{
   majorVer = INF_ISECT_STRATEGY_PLUGIN_API_MAJOR;
   minorVer = INF_ISECT_STRATEGY_PLUGIN_API_MINOR;
}

IOV_PLUGIN_API(inf::PluginCreatorBase*) getIntersectionStrategyCreator()
{
   return &sPluginCreator;
}
//@}

}

namespace inf
{

void PointIntersectionStrategy::init(ViewerPtr viewer)
{
   mGrabData = viewer->getSceneObj()->getSceneData<GrabData>();
}

OSG::TransformNodePtr PointIntersectionStrategy::findIntersection(ViewerPtr viewer, gmtl::Point3f& intersectPoint)
{
   intersectPoint.set(0.0f, 0.0f, 0.0f);

   WandInterfacePtr wand = viewer->getUser()->getInterfaceTrader().getWandInterface();
   const gmtl::Matrix44f vp_M_wand_xform(
      wand->getWandPos()->getData(viewer->getDrawScaleFactor())
   );

   OSG::TransformNodePtr intersect_obj;

   const GrabData::object_list_t& objects = mGrabData->getObjects();

   // Find the first object in objects with which the wand intersects.
   GrabData::object_list_t::const_iterator o;
   for ( o = objects.begin(); o != objects.end(); ++o )
   {
      OSG::Matrix world_xform;
      (*o).node()->getParent()->getToWorld(world_xform);
      gmtl::Matrix44f obj_M_vp;
      gmtl::set(obj_M_vp, world_xform);
      gmtl::invert(obj_M_vp);

      // Get the wand transformation in virtual world coordinates,
      // including any transformations in the scene graph below the
      // transformation root.
      const gmtl::Matrix44f obj_M_wand_xform = obj_M_vp * vp_M_wand_xform;
      const gmtl::Vec3f wand_pos_vw(gmtl::makeTrans<gmtl::Vec3f>(obj_M_wand_xform));
      const OSG::Pnt3f wand_point(wand_pos_vw[0], wand_pos_vw[1],
                                  wand_pos_vw[2]);

      const OSG::DynamicVolume& bbox = (*o).node()->getVolume();

      if ( bbox.intersect(wand_point) )
      {
         intersect_obj = *o;
         intersectPoint.set(wand_point.getValues());
         break;
      }
   }
   return intersect_obj;
}

}
