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

#ifndef _VRKIT_RAY_INTERSECTION_STRATEGY_H_
#define _VRKIT_RAY_INTERSECTION_STRATEGY_H_

#include <string>

#include <boost/enable_shared_from_this.hpp>
#include <boost/signals/connection.hpp>

#include <OpenSG/OSGColor.h>
#include <OpenSG/OSGLine.h>
#include <OpenSG/OSGGeometry.h>
#include <OpenSG/OSGSwitch.h>
#include <OpenSG/OSGGeometry.h>
#include <OpenSG/OSGNode.h>

#include <gmtl/Point.h>

#include <jccl/Config/ConfigElementPtr.h>

#include <vrkit/ViewerPtr.h>
#include <vrkit/isect/Strategy.h>
#include <vrkit/util/SceneObjectTraverser.h>


namespace vrkit
{

class RayIntersectionStrategy
   : public isect::Strategy
   , public boost::enable_shared_from_this<RayIntersectionStrategy>
{
protected:
   RayIntersectionStrategy(const plugin::Info& info);

public:
   virtual ~RayIntersectionStrategy();

   static std::string getId()
   {
      return "RayIntersection";
   }

   static isect::StrategyPtr create(const plugin::Info& info)
   {
      return isect::StrategyPtr(new RayIntersectionStrategy(info));
   }

   virtual isect::StrategyPtr init(ViewerPtr viewer);

   virtual void update(ViewerPtr viewer);

   virtual SceneObjectPtr findIntersection(
      ViewerPtr viewer, const std::vector<SceneObjectPtr>& objs,
      gmtl::Point3f& intersectPoint
   );

   void setVisible(const bool visible);

   void initGeom();

private:
   static std::string getElementType()
   {
      return "ray_intersection_strategy";
   }

   /** @name Intersection Traversal Methods */
   //@{
   util::SceneObjectTraverser::Result enterFunc(SceneObjectPtr obj);
   void setHit(float enterVal, SceneObjectPtr obj, const OSG::Pnt3f&  point);
   //@}

   /**
    * Configures this intersection strategy.
    *
    * @pre The type of the given config element matches the identifier
    *      returned by getElementType().
    *
    * @param cfgElt The config element to use for configuring this object.
    *
    * @throw vrkit::PluginException
    *           Thrown if the version of the given config element is too old.
    */
   void configure(jccl::ConfigElementPtr cfgElt);

   OSG::Line            mSelectionRay;    /**< The ray used for selection. */

   // The points used for visualising the ray and hit object
   OSG::GeoPositions3fPtr    mGeomPts;

   // The visualisation geometry, needed for update.
   OSG::GeometryNodePtr mGeomNode;
   OSG::SwitchNodePtr   mSwitchNode;

   boost::signals::connection mRayIsectConn;	/**< Visibility connection object. */

   /** @name Ray Properties */
   //@{
   float        mRayLength;     /**< The length of the rendered "ray" */
   OSG::Color4f mRayDiffuse;
   OSG::Color4f mRayAmbient;
   OSG::Real32  mRayWidth;
   //@}

   /** @name Intersection Traversal Properties */
   //@{
   float                mMinDist;
   SceneObjectPtr       mIntersectObj;
   OSG::Pnt3f           mIntersectPoint;
   gmtl::Matrix44f      m_vp_M_wand;
   //@}

   bool mTriangleIsect;
};

}


#endif /* _VRKIT_RAY_INTERSECTION_STRATEGY_H_ */
