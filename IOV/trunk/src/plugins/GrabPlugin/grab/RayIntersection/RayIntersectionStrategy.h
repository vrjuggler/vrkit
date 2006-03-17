// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_RAY_INTERSECTION_STRATEGY_H_
#define _INF_RAY_INTERSECTION_STRATEGY_H_

#include <boost/enable_shared_from_this.hpp>
#include <OpenSG/OSGTransform.h>
#include <IOV/ViewerPtr.h>
#include <IOV/GrabDataPtr.h>
#include "../../IntersectionStrategy.h"

#include <gmtl/Point.h>

#include <OpenSG/OSGLine.h>
#include <OpenSG/OSGGeometry.h>
#include <OpenSG/OSGSwitch.h>
#include <OpenSG/OSGGeometry.h>
#include <OpenSG/OSGNode.h>

#include <string>

namespace inf
{

class RayIntersectionStrategy
   : public IntersectionStrategy
   , public boost::enable_shared_from_this<RayIntersectionStrategy>
{
protected:
   RayIntersectionStrategy()
      : inf::IntersectionStrategy()
   {;}

public:
   virtual ~RayIntersectionStrategy()
   {;}

   static std::string getId()
   {
      return "RayIntersection";
   }

   static IntersectionStrategyPtr create()
   {
      return IntersectionStrategyPtr(new RayIntersectionStrategy());
   }

   virtual void init(ViewerPtr viewer);

   virtual void update(ViewerPtr viewer);

   virtual OSG::TransformNodePtr findIntersection(ViewerPtr viewer, gmtl::Point3f& intersectPoint);

   void setVisible(bool visible);
   void initGeom();
private:
   inf::GrabDataPtr mGrabData;

   OSG::Line            mSelectionRay;    /**< The ray used for selection. */

   // The points used for visualising the ray and hit object
   OSG::GeoPositions3fPtr    mGeomPts;

   // The visualisation geometry, needed for update.
   OSG::GeometryNodePtr mGeomNode;
   OSG::SwitchNodePtr   mSwitchNode;

   float                mRayLength;
};

}
#endif /*_INF_POINT_INTERSECTION_STRATEGY_H_*/
