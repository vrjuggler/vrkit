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

#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/assign/list_of.hpp>

#include <gmtl/Matrix.h>
#include <gmtl/External/OpenSGConvert.h>

#include <vrkit/plugin/Config.h>
#include <vrkit/InterfaceTrader.h>
#include <vrkit/User.h>
#include <vrkit/Viewer.h>
#include <vrkit/WandInterface.h>
#include <vrkit/SceneObject.h>
#include <vrkit/Version.h>
#include <vrkit/plugin/Creator.h>
#include <vrkit/plugin/Info.h>
#include <vrkit/util/OpenSGHelpers.h>

#include "PointIntersectionStrategy.h"


using namespace boost::assign;

static const vrkit::plugin::Info sInfo(
   "com.infiscape.isect", "PointIntersectionStrategy",
   list_of(VRKIT_VERSION_MAJOR)(VRKIT_VERSION_MINOR)(VRKIT_VERSION_PATCH)
);
static vrkit::plugin::Creator<vrkit::isect::Strategy> sPluginCreator(
   boost::bind(&vrkit::PointIntersectionStrategy::create, sInfo)
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

namespace vrkit
{

isect::StrategyPtr PointIntersectionStrategy::init(ViewerPtr)
{
   return shared_from_this();
}

SceneObjectPtr PointIntersectionStrategy::
findIntersection(ViewerPtr viewer, const std::vector<SceneObjectPtr>& objs,
                 gmtl::Point3f& intersectPoint)
{
   WandInterfacePtr wand =
      viewer->getUser()->getInterfaceTrader().getWandInterface();
   const gmtl::Matrix44f vp_M_wand_xform(
      wand->getWandPos()->getData(viewer->getDrawScaleFactor())
   );

   // Reset the intersection test data.
   mIntersectObj   = SceneObjectPtr();
   mIntersectPoint = OSG::Pnt3f();

   // Traverse all the given scene objects to find the first intersection.
   util::SceneObjectTraverser::traverse(
      objs, boost::bind(&PointIntersectionStrategy::enter, this, _1,
                        boost::cref(vp_M_wand_xform))
   );

   // If there was an intersection, this updates intersectPoint to the point
   // of intersection. Otherwise, it accurately sets it to be a zeroed-out
   // point.
   intersectPoint.set(mIntersectPoint.getValues());

   return mIntersectObj;
}

util::SceneObjectTraverser::Result PointIntersectionStrategy::
enter(SceneObjectPtr obj, const gmtl::Matrix44f& vp_M_wand)
{
   // The default behavior is to continue with the traversal. If obj cannot
   // be intersected, one or more of its children may allow intersection.
   util::SceneObjectTraverser::Result result =
      util::SceneObjectTraverser::Continue;

   if ( obj->canIntersect() )
   {
      OSG::Matrix world_xform;

      vprASSERT(obj->getRoot() != OSG::NullFC);
      OSG::NodeRefPtr root(obj->getRoot());

      // If we have no parent then we want to use the identity.
      if ( root->getParent() != OSG::NullFC )
      {
         root->getParent()->getToWorld(world_xform);
      }

      gmtl::Matrix44f obj_M_vp;
      gmtl::set(obj_M_vp, world_xform);
      gmtl::invert(obj_M_vp);

      // Get the wand transformation in virtual world coordinates, including
      // any transformations in the scene graph below the transformation root.
      const gmtl::Matrix44f obj_M_wand_xform = obj_M_vp * vp_M_wand;
      const gmtl::Vec3f wand_pos_vw(
         gmtl::makeTrans<gmtl::Vec3f>(obj_M_wand_xform)
      );
      const OSG::Pnt3f wand_point(wand_pos_vw[0], wand_pos_vw[1],
                                  wand_pos_vw[2]);

      if ( root->getVolume().intersect(wand_point) )
      {
         mIntersectObj = obj;
         mIntersectPoint.setValue(wand_point.getValues());

         // If obj has no children, then we have found the object that will
         // be intersected.
         if ( ! obj->hasChildren() )
         {
            result = util::SceneObjectTraverser::Stop;
         }
      }
      // If there is no intersection with obj, then we do not need to search
      // its children since they would be included with encompassing bounding
      // box.
      else
      {
         result = util::SceneObjectTraverser::Skip;
      }
   }

   return result;
}

}
