// Copyright (C) Infiscape Corporation 2005-2006

#include <OpenSG/OSGChunkMaterial.h>
#include <OpenSG/OSGMaterialChunk.h>
#include <OpenSG/OSGLineChunk.h>

#include <gmtl/Matrix.h>
#include <gmtl/External/OpenSGConvert.h>

#include <jccl/Config/ConfigElement.h>

#include <IOV/GrabData.h>
#include <IOV/InterfaceTrader.h>
#include <IOV/Plugin/PluginConfig.h>
#include <IOV/PluginCreator.h>
#include <IOV/User.h>
#include <IOV/Viewer.h>
#include <IOV/WandInterface.h>

#include "RayIntersectionStrategy.h"


static inf::PluginCreator<inf::IntersectionStrategy> sPluginCreator(
   &inf::RayIntersectionStrategy::create, "Ray Intersection Strategy Plug-in"
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

RayIntersectionStrategy::RayIntersectionStrategy()
   : inf::IntersectionStrategy()
   , mRayLength(20.0f)
   , mRayDiffuse(1.0f, 0.0f, 0.0f, 1.0f)
   , mRayAmbient(1.0f, 0.0f, 0.0f, 1.0f)
   , mRayWidth(5.0f)
{
   /* Do nothing. */ ;
}

void RayIntersectionStrategy::init(ViewerPtr viewer)
{
   mGrabData = viewer->getSceneObj()->getSceneData<GrabData>();

   jccl::ConfigElementPtr cfg_elt =
      viewer->getConfiguration().getConfigElement(getElementType());

   if ( cfg_elt )
   {
      try
      {
         configure(cfg_elt);
      }
      catch (inf::Exception& ex)
      {
         std::cerr << ex.what() << std::endl;
      }
   }

   // Scale the ray length (measured in feet) into application units.
   mRayLength *= 0.3048f * viewer->getDrawScaleFactor();

   initGeom();

   OSG::GroupNodePtr decorator_root = viewer->getSceneObj()->getDecoratorRoot();
   OSG::beginEditCP(decorator_root.node());
      decorator_root.node()->addChild(mSwitchNode);
   OSG::endEditCP(decorator_root.node());
   setVisible(true);
}

void RayIntersectionStrategy::update(ViewerPtr viewer)
{
   WandInterfacePtr wand = viewer->getUser()->getInterfaceTrader().getWandInterface();
   const gmtl::Matrix44f vp_M_wand(
      wand->getWandPos()->getData(viewer->getDrawScaleFactor())
   );

   gmtl::Rayf wand_ray(gmtl::Vec3f(0,0,0), gmtl::Vec3f(0,0,-1));
   gmtl::xform(wand_ray, vp_M_wand, wand_ray);

   mSelectionRay.setValue(OSG::Pnt3f(wand_ray.mOrigin.getData()),
                          OSG::Vec3f(wand_ray.mDir.getData()));

   OSG::Pnt3f start_pt, end_pt;
   start_pt = mSelectionRay.getPosition();
   end_pt = start_pt + mSelectionRay.getDirection() * mRayLength;

   OSG::beginEditCP(mGeomPts);
      mGeomPts->setValue(start_pt,0);
      mGeomPts->setValue(  end_pt,1);
   OSG::endEditCP(mGeomPts);
}

void RayIntersectionStrategy::setVisible(bool visible)
{
   OSG::beginEditCP(mSwitchNode);
   if(visible)
   {
      mSwitchNode->setChoice(OSG::Switch::ALL);
   }
   else
   {
      mSwitchNode->setChoice(OSG::Switch::NONE);
   }
   OSG::endEditCP(mSwitchNode);
}

void RayIntersectionStrategy::initGeom()
{
   // Create a small geometry to show the ray and what was hit
   // Contains a line and a single triangle.
   // The line shows the ray, the triangle whatever was hit.
   OSG::ChunkMaterialPtr chunk_mat = OSG::ChunkMaterial::create();
   OSG::MaterialChunkPtr mat_chunk = OSG::MaterialChunk::create();
   OSG::LineChunkPtr line_chunk = OSG::LineChunk::create();

   OSG::CPEditor cme(chunk_mat);
   OSG::CPEditor mce(mat_chunk);
   OSG::CPEditor lce(line_chunk);
   mat_chunk->setLit(true);
   mat_chunk->setDiffuse(mRayDiffuse);
   mat_chunk->setAmbient(mRayAmbient);
   line_chunk->setWidth(mRayWidth);
   chunk_mat->addChunk(mat_chunk);
   chunk_mat->addChunk(line_chunk);

   mGeomPts = OSG::GeoPositions3f::create();
   OSG::beginEditCP(mGeomPts);
   {
     mGeomPts->addValue(OSG::Pnt3f(0,0,0));
     mGeomPts->addValue(OSG::Pnt3f(0,0,0));
   }
   OSG::endEditCP(mGeomPts);

   OSG::GeoIndicesUI32Ptr index = OSG::GeoIndicesUI32::create();
   OSG::beginEditCP(index);
   {
     index->addValue(0);
     index->addValue(1);
   }
   OSG::endEditCP(index);

   OSG::GeoPLengthsUI32Ptr lens = OSG::GeoPLengthsUI32::create();
   OSG::beginEditCP(lens);
   {
     lens->addValue(2);
   }
   OSG::endEditCP(lens);

   OSG::GeoPTypesUI8Ptr type = OSG::GeoPTypesUI8::create();
   OSG::beginEditCP(type);
   {
     type->addValue(GL_LINES);
   }
   OSG::endEditCP(type);

   mGeomNode = OSG::GeometryNodePtr::create();

   OSG::beginEditCP(mGeomNode);
   {
     mGeomNode->setPositions(mGeomPts);
     mGeomNode->setIndices(index);
     mGeomNode->setLengths(lens);
     mGeomNode->setTypes(type);
     mGeomNode->setMaterial(chunk_mat);
   }
   OSG::endEditCP(mGeomNode);

   mSwitchNode = OSG::SwitchNodePtr::create();
   OSG::beginEditCP(mSwitchNode);
      mSwitchNode.node()->addChild(mGeomNode);
      mSwitchNode->setChoice(OSG::Switch::ALL);
   OSG::endEditCP(mSwitchNode);
}

OSG::TransformNodePtr RayIntersectionStrategy::findIntersection(ViewerPtr viewer, gmtl::Point3f& intersectPoint)
{
   WandInterfacePtr wand = viewer->getUser()->getInterfaceTrader().getWandInterface();
   const gmtl::Matrix44f vp_M_wand(
      wand->getWandPos()->getData(viewer->getDrawScaleFactor())
   );

   OSG::TransformNodePtr intersect_obj;

   float min_dist = 999999999.9f;   // Set to a max
   const GrabData::object_list_t& objects = mGrabData->getObjects();
   OSG::Line osg_pick_ray;
   OSG::Pnt3f osg_intersect_point;

   GrabData::object_list_t::const_iterator o;
   for ( o = objects.begin(); o != objects.end(); ++o)
   {
      OSG::Matrix world_xform;
      (*o).node()->getParent()->getToWorld(world_xform);
      gmtl::Matrix44f obj_M_vp;
      gmtl::set(obj_M_vp, world_xform);
      gmtl::invert(obj_M_vp);

      // Get the wand transformation in virtual world coordinates,
      // including any transformations in the scene graph below the
      // transformation root.
      const gmtl::Matrix44f obj_M_wand = obj_M_vp * vp_M_wand;
      gmtl::Rayf pick_ray(gmtl::Vec3f(0,0,0), gmtl::Vec3f(0,0,-1));
      gmtl::xform(pick_ray, obj_M_wand, pick_ray);
      osg_pick_ray.setValue(OSG::Pnt3f(pick_ray.mOrigin.getData()),
                            OSG::Vec3f(pick_ray.mDir.getData()));
       
      float enter_val, exit_val;
      if(((*o).node() != OSG::NullFC) &&
         ((*o).node()->getVolume().intersect(osg_pick_ray, enter_val, exit_val)) )
      {
         if (enter_val < min_dist)
         {
            intersect_obj = *o;
            min_dist = enter_val;
            osg_intersect_point = osg_pick_ray.getPosition() + (min_dist * osg_pick_ray.getDirection());
         }
      }

   }

   intersectPoint.set(osg_intersect_point.getValues());
   return intersect_obj;
}

void RayIntersectionStrategy::configure(jccl::ConfigElementPtr cfgElt)
   throw (inf::Exception)
{
   vprASSERT(cfgElt->getID() == getElementType());

   const unsigned int req_cfg_version(1);

   if ( cfgElt->getVersion() < req_cfg_version )
   {
      std::stringstream msg;
      msg << "Configuration of RayIntersectionStrategy failed.  Required "
          << "config element version is " << req_cfg_version
          << ", but element '" << cfgElt->getName() << "' is version "
          << cfgElt->getVersion();
      throw inf::PluginException(msg.str(), IOV_LOCATION);
   }

   const std::string ray_length_prop("ray_length");
   const std::string ray_width_prop("ray_width");
   const std::string ray_diffuse_prop("ray_diffuse_color");
   const std::string ray_ambient_prop("ray_ambient_color");

   const float ray_len = cfgElt->getProperty<float>(ray_length_prop);

   if ( ray_len > 0.0f )
   {
      mRayLength = ray_len;
   }

   const OSG::Real32 ray_width(
      cfgElt->getProperty<OSG::Real32>(ray_width_prop)
   );

   if ( ray_width > 0.0f )
   {
      mRayWidth = ray_width;
   }

   OSG::Real32 red, green, blue;

   red   = cfgElt->getProperty<OSG::Real32>(ray_diffuse_prop, 0);
   green = cfgElt->getProperty<OSG::Real32>(ray_diffuse_prop, 1);
   blue  = cfgElt->getProperty<OSG::Real32>(ray_diffuse_prop, 2);

   if ( 0.0f <= red && red <= 1.0f )
   {
      mRayDiffuse[0] = red;
   }

   if ( 0.0f <= green && green <= 1.0f )
   {
      mRayDiffuse[1] = green;
   }

   if ( 0.0f <= blue && blue <= 1.0f )
   {
      mRayDiffuse[2] = blue;
   }

   red   = cfgElt->getProperty<float>(ray_ambient_prop, 0);
   green = cfgElt->getProperty<float>(ray_ambient_prop, 1);
   blue  = cfgElt->getProperty<float>(ray_ambient_prop, 2);

   if ( 0.0f <= red && red <= 1.0f )
   {
      mRayAmbient[0] = red;
   }

   if ( 0.0f <= green && green <= 1.0f )
   {
      mRayAmbient[1] = green;
   }

   if ( 0.0f <= blue && blue <= 1.0f )
   {
      mRayAmbient[2] = blue;
   }
}

}
