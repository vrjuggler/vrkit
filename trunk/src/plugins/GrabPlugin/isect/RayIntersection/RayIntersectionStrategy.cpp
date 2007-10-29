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

#include <limits>
#include <boost/bind.hpp>
#include <boost/cast.hpp>
#include <boost/assign/list_of.hpp>

#include <OpenSG/OSGChunkMaterial.h>
#include <OpenSG/OSGMaterialChunk.h>
#include <OpenSG/OSGLineChunk.h>
#include <OpenSG/OSGIntersectAction.h>
#include <OpenSG/OSGTriangleIterator.h>

#include <gmtl/Intersection.h>
#include <gmtl/Matrix.h>
#include <gmtl/External/OpenSGConvert.h>

#include <jccl/Config/ConfigElement.h>

#include <vrkit/plugin/Config.h>
#include <vrkit/InterfaceTrader.h>
#include <vrkit/User.h>
#include <vrkit/Viewer.h>
#include <vrkit/WandInterface.h>
#include <vrkit/SceneObject.h>
#include <vrkit/Version.h>
#include <vrkit/signal/Repository.h>
#include <vrkit/plugin/Creator.h>
#include <vrkit/plugin/Info.h>
#include <vrkit/exceptions/PluginException.h>

#include "RayIntersectionStrategy.h"


using namespace boost::assign;

static const vrkit::plugin::Info sInfo(
   "com.infiscape.isect", "RayIntersectionStrategy",
   list_of(VRKIT_VERSION_MAJOR)(VRKIT_VERSION_MINOR)(VRKIT_VERSION_PATCH)
);
static vrkit::plugin::Creator<vrkit::isect::Strategy> sPluginCreator(
   boost::bind(&vrkit::RayIntersectionStrategy::create, sInfo)
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
   majorVer = VRKIT_ISECT_STRATEGY_PLUGIN_API_MAJOR;
   minorVer = VRKIT_ISECT_STRATEGY_PLUGIN_API_MINOR;
}

VRKIT_PLUGIN_API(vrkit::plugin::CreatorBase*) getIntersectionStrategyCreator()
{
   return &sPluginCreator;
}
//@}

}

namespace
{

#if OSG_MAJOR_VERSION < 2
typedef OSG::CNodePtr& enter_t;
#else
typedef OSG::NodeCorePtrConstArg enter_t;
#endif

OSG::Action::ResultE geometryEnter(enter_t nodeCore, OSG::Action* action)
{
   OSG::IntersectAction* ia =
      boost::polymorphic_downcast<OSG::IntersectAction*>(action);
   OSG::NodePtr hit_node(ia->getHitObject());
   OSG::GeometryPtr core =
#if OSG_MAJOR_VERSION < 2
      OSG::GeometryPtr::dcast(hit_node->getCore());
//   OSG::Geometry* core =
//      boost::polymorphic_downcast<OSG::Geometry*>(
//         hit_node->getCore().getCPtr()
//      );
#else
      OSG::cast_dynamic<OSG::GeometryPtr>(nodeCore);
#endif

   OSG::TriangleIterator it;

   for ( it = core->beginTriangles(); it != core->endTriangles(); ++it )
   {
      OSG::Real32 t;
      OSG::Vec3f norm;

      if ( ia->getLine().intersect(it.getPosition(0), it.getPosition(1),
                                   it.getPosition(2), t, &norm) )
      {
#if OSG_MAJOR_VERSION < 2
         ia->setHit(t, hit_node, it.getIndex(), norm);
#else
         ia->setHit(t, hit_node, it.getIndex(), norm, ia->getHitLine());
#endif
      }
   }

   return OSG::Action::Continue;
}

}

namespace vrkit
{

RayIntersectionStrategy::RayIntersectionStrategy(const plugin::Info& info)
   : isect::Strategy(info)
   , mRayLength(20.0f)
   , mRayDiffuse(1.0f, 0.0f, 0.0f, 1.0f)
   , mRayAmbient(1.0f, 0.0f, 0.0f, 1.0f)
   , mRayWidth(5.0f)
   , mTriangleIsect(false)
{
   /* Do nothing. */ ;
}

RayIntersectionStrategy::~RayIntersectionStrategy()
{
   mRayIsectConn.disconnect();
}

isect::StrategyPtr RayIntersectionStrategy::init(ViewerPtr viewer)
{
   jccl::ConfigElementPtr cfg_elt =
      viewer->getConfiguration().getConfigElement(getElementType());

   if ( cfg_elt )
   {
      try
      {
         configure(cfg_elt);
      }
      catch (Exception& ex)
      {
         std::cerr << ex.what() << std::endl;
      }
   }

   // If we are going to do triangle-level intersections, then register our
   // geometry core callback with OSG::IntersectAction.
   if ( mTriangleIsect )
   {
      OSG::IntersectAction::registerEnterDefault(
         OSG::Geometry::getClassType(),
#if OSG_MAJOR_VERSION < 2
         OSG::osgTypedFunctionFunctor2CPtrRef<
            OSG::Action::ResultE, OSG::CNodePtr, OSG::Action*
         >(geometryEnter)
#else
         &geometryEnter
#endif
      );
   }

   // Scale the ray length (measured in feet) into application units.
   mRayLength *= 0.3048f * viewer->getDrawScaleFactor();

   initGeom();

   OSG::GroupNodePtr decorator_root =
      viewer->getSceneObj()->getDecoratorRoot();
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor dre(decorator_root.node(), OSG::Node::ChildrenFieldMask);
#endif
   decorator_root.node()->addChild(mSwitchNode);
   setVisible(true);

   // Register visibility signal with vrkit::signal::Repository.
   signal::RepositoryPtr sig_repos =
      viewer->getSceneObj()->getSceneData<signal::Repository>();

   typedef boost::signal<void (bool)> sig_type;
   std::string sig_name("Set Intersection Ray Visibility");

   if ( ! sig_repos->hasSignal(sig_name) )
   {
      sig_repos->addSignal(sig_name, signal::Container<sig_type>::create());
   }

   // Connect new signal to slot after SwitchNode creation
   mRayIsectConn =
      sig_repos->getSignal<sig_type>(sig_name)->connect(
         boost::bind(&RayIntersectionStrategy::setVisible, this, _1)
      );

   return shared_from_this();
}

void RayIntersectionStrategy::update(ViewerPtr viewer)
{
   WandInterfacePtr wand =
      viewer->getUser()->getInterfaceTrader().getWandInterface();
   const gmtl::Matrix44f vp_M_wand(
      wand->getWandPos()->getData(viewer->getDrawScaleFactor())
   );

   gmtl::Rayf wand_ray(gmtl::Vec3f(0.0f, 0.0f, 0.0f),
                       gmtl::Vec3f(0.0f, 0.0f, -1.0f));
   gmtl::xform(wand_ray, vp_M_wand, wand_ray);

   mSelectionRay.setValue(OSG::Pnt3f(wand_ray.mOrigin.getData()),
                          OSG::Vec3f(wand_ray.mDir.getData()));

   OSG::Pnt3f start_pt, end_pt;
   start_pt = mSelectionRay.getPosition();
   end_pt = start_pt + mSelectionRay.getDirection() * mRayLength;

#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor gpe(mGeomPts);
#endif
   mGeomPts->setValue(start_pt, 0);
   mGeomPts->setValue(  end_pt, 1);
}

void RayIntersectionStrategy::setVisible(const bool visible)
{
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor snce(mSwitchNode.core(), OSG::Switch::ChoiceFieldMask);
#endif

   if ( visible )
   {
      mSwitchNode->setChoice(OSG::Switch::ALL);
   }
   else
   {
      mSwitchNode->setChoice(OSG::Switch::NONE);
   }
}

void RayIntersectionStrategy::initGeom()
{
   // Create a small geometry to show the ray and what was hit
   // Contains a line and a single triangle.
   // The line shows the ray, the triangle whatever was hit.
   OSG::ChunkMaterialPtr chunk_mat = OSG::ChunkMaterial::create();
   OSG::MaterialChunkPtr mat_chunk = OSG::MaterialChunk::create();
   OSG::LineChunkPtr line_chunk = OSG::LineChunk::create();

#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor cme(chunk_mat);
   OSG::CPEditor mce(mat_chunk);
   OSG::CPEditor lce(line_chunk);
#endif

   mat_chunk->setLit(true);
   mat_chunk->setDiffuse(mRayDiffuse);
   mat_chunk->setAmbient(mRayAmbient);
   line_chunk->setWidth(mRayWidth);
   chunk_mat->addChunk(mat_chunk);
   chunk_mat->addChunk(line_chunk);

   mGeomPts = OSG::GeoPositions3f::create();
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor gpe(mGeomPts);
#endif
   mGeomPts->addValue(OSG::Pnt3f(0.0f, 0.0f, 0.0f));
   mGeomPts->addValue(OSG::Pnt3f(0.0f, 0.0f, 0.0f));

   OSG::GeoIndicesUI32Ptr index = OSG::GeoIndicesUI32::create();
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor ie(index);
#endif
   index->addValue(0);
   index->addValue(1);

   OSG::GeoPLengthsUI32Ptr lens = OSG::GeoPLengthsUI32::create();
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor le(lens);
#endif
   lens->addValue(2);

   OSG::GeoPTypesUI8Ptr type = OSG::GeoPTypesUI8::create();
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor te(type);
#endif
   type->addValue(GL_LINES);

   mGeomNode = OSG::GeometryNodePtr::create();

#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor gnce(mGeomNode.core());
#endif
   mGeomNode->setPositions(mGeomPts);
   mGeomNode->setIndices(index);
   mGeomNode->setLengths(lens);
   mGeomNode->setTypes(type);
   mGeomNode->setMaterial(chunk_mat);

   mSwitchNode = OSG::SwitchNodePtr::create();
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor sne(mSwitchNode.node(), OSG::Node::ChildrenFieldMask);
   OSG::CPEditor snce(mSwitchNode.core(), OSG::Switch::ChoiceFieldMask);
#endif
   mSwitchNode.node()->addChild(mGeomNode);
   mSwitchNode->setChoice(OSG::Switch::ALL);
}

SceneObjectPtr RayIntersectionStrategy::
findIntersection(ViewerPtr viewer, const std::vector<SceneObjectPtr>& objs,
                 gmtl::Point3f& intersectPoint)
{
   WandInterfacePtr wand =
      viewer->getUser()->getInterfaceTrader().getWandInterface();
   const gmtl::Matrix44f vp_M_wand(
      wand->getWandPos()->getData(viewer->getDrawScaleFactor())
   );

   // Reset the intersection test data.
   m_vp_M_wand = vp_M_wand;
   mMinDist = std::numeric_limits<float>::max();
   mIntersectObj = SceneObjectPtr();
   mIntersectPoint = OSG::Pnt3f();

   // Traverse scene objects to find closest intersection.
   util::SceneObjectTraverser::enter_func_t enter =
      boost::bind(&RayIntersectionStrategy::enterFunc, this, _1);
   util::SceneObjectTraverser::traverse(objs, enter);

   intersectPoint.set(mIntersectPoint.getValues());
   return mIntersectObj;
}

util::SceneObjectTraverser::Result RayIntersectionStrategy::
enterFunc(SceneObjectPtr obj)
{
   if ( ! obj->canIntersect() )
   {
      return util::SceneObjectTraverser::Continue;
   }

   OSG::Matrix world_xform;

   vprASSERT(obj->getRoot() != OSG::NullFC);

   // If we have no parent then we want to use the identity.
   if ( obj->getRoot()->getParent() != OSG::NullFC )
   {
      obj->getRoot()->getParent()->getToWorld(world_xform);
   }

   gmtl::Matrix44f obj_M_vp;
   gmtl::set(obj_M_vp, world_xform);
   gmtl::invert(obj_M_vp);

   // Get the wand transformation in virtual world coordinates, including any
   // transformations in the scene graph below the transformation root.
   const gmtl::Matrix44f obj_M_wand = obj_M_vp * m_vp_M_wand;
   gmtl::Rayf pick_ray(gmtl::Vec3f(0.0f, 0.0f, 0.0f),
                       gmtl::Vec3f(0.0f, 0.0f, -1.0f));
   gmtl::xform(pick_ray, obj_M_wand, pick_ray);

   OSG::NodeRefPtr root = obj->getRoot();

   OSG::Pnt3f vol_min, vol_max;
   root->getVolume().getBounds(vol_min, vol_max);

   gmtl::AABoxf bbox;
   bbox.setMin(gmtl::Point3f(vol_min[0], vol_min[1], vol_min[2]));
   bbox.setMax(gmtl::Point3f(vol_max[0], vol_max[1], vol_max[2]));
   unsigned int num_hits;
   float enter_val, exit_val;

   util::SceneObjectTraverser::Result result =
      util::SceneObjectTraverser::Skip;

   // Use a GMTL shell intersection test rather than the OpenSG volume
   // intersection test. Using the shell intersection provides better results
   // when, for example, the origin of the pick ray is inside a large volume
   // that contains smaller volumes.
   if ( gmtl::intersect(bbox, pick_ray, num_hits, enter_val, exit_val) )
   {
      // Intersected bounding volume so we must continue into children.
      result = util::SceneObjectTraverser::Continue;

      const OSG::Line osg_pick_ray(
         OSG::Pnt3f(pick_ray.mOrigin.getData()),
         OSG::Vec3f(pick_ray.mDir.getData())
      );

      // If we are doing triangle-level intersection, then we create an
      // intersect action and apply it to the intersected object. Earlier, a
      // callback was registered for handling geometry cores to perform the
      // triangle intersection test.
      if ( mTriangleIsect )
      {
         // Temporarily allow intersection traversal into current scene
         // object.
         OSG::UInt32 trav_mask = obj->getRoot()->getTravMask();
#if OSG_MAJOR_VERSION < 2
         OSG::CPEditor ore(obj->getRoot(), OSG::Node::TravMaskFieldMask);
#endif
         obj->getRoot()->setTravMask(trav_mask | 128);

         OSG::IntersectAction* action(OSG::IntersectAction::create());
         action->setTravMask(128);

         action->setLine(osg_pick_ray);
         action->apply(root);

         // If we got a hit, then we update the state of our intersection
         // test.
         if ( action->didHit() )
         {
            enter_val = action->getHitT();
            setHit(enter_val, obj, action->getHitPoint());
         }

         // Disable intersection traversal into current scene object.
         trav_mask = obj->getRoot()->getTravMask();
         obj->getRoot()->setTravMask(trav_mask & ~128);

         delete action;
      }
      // If enter_val is less than mMinDist, then our ray has intersected an
      // object that is closer than the last intersected object.
      else
      {
         // If we are not doing triangle-level intersection, then intersecting
         // with the bounding volume is sufficient to have an object
         // intersection.
         OSG::Pnt3f intersect_point = osg_pick_ray.getPosition() +
            enter_val * osg_pick_ray.getDirection();
         setHit(enter_val, obj, intersect_point);
      }
   }

   return result;
}

void RayIntersectionStrategy::setHit(float enterVal, SceneObjectPtr obj,
                                     const OSG::Pnt3f& point)
{
   if ( enterVal < mMinDist )
   {
      mMinDist = enterVal;
      mIntersectObj = obj;
      mIntersectPoint = point;
   }
}

void RayIntersectionStrategy::configure(jccl::ConfigElementPtr cfgElt)
{
   vprASSERT(cfgElt->getID() == getElementType());

   const unsigned int req_cfg_version(2);

   if ( cfgElt->getVersion() < req_cfg_version )
   {
      std::stringstream msg;
      msg << "Configuration of RayIntersectionStrategy failed.  Required "
          << "config element version is " << req_cfg_version
          << ", but element '" << cfgElt->getName() << "' is version "
          << cfgElt->getVersion();
      throw PluginException(msg.str(), VRKIT_LOCATION);
   }

   const std::string ray_length_prop("ray_length");
   const std::string ray_width_prop("ray_width");
   const std::string ray_diffuse_prop("ray_diffuse_color");
   const std::string ray_ambient_prop("ray_ambient_color");
   const std::string tri_isect_prop("triangle_intersect");

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

   mTriangleIsect = cfgElt->getProperty<bool>(tri_isect_prop);
}

}
