// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_RAY_INTERSECTION_STRATEGY_H_
#define _INF_RAY_INTERSECTION_STRATEGY_H_

#include <string>

#include <boost/enable_shared_from_this.hpp>

#include <OpenSG/OSGColor.h>
#include <OpenSG/OSGTransform.h>
#include <OpenSG/OSGLine.h>
#include <OpenSG/OSGGeometry.h>
#include <OpenSG/OSGSwitch.h>
#include <OpenSG/OSGGeometry.h>
#include <OpenSG/OSGNode.h>

#include <gmtl/Point.h>

#include <jccl/Config/ConfigElementPtr.h>

#include <IOV/ViewerPtr.h>
#include <IOV/Util/Exceptions.h>
#include <IOV/Grab/IntersectionStrategy.h>


namespace inf
{

class RayIntersectionStrategy
   : public IntersectionStrategy
   , public boost::enable_shared_from_this<RayIntersectionStrategy>
{
protected:
   RayIntersectionStrategy();

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

   virtual OSG::TransformNodePtr
      findIntersection(ViewerPtr viewer, const std::vector<OSG::TransformNodePtr>& objs, gmtl::Point3f& intersectPoint);

   void setVisible(bool visible);

   void initGeom();

private:
   static std::string getElementType()
   {
      return "ray_intersection_strategy";
   }

   void configure(jccl::ConfigElementPtr cfgElt) throw (inf::Exception);

   OSG::Line            mSelectionRay;    /**< The ray used for selection. */

   // The points used for visualising the ray and hit object
   OSG::GeoPositions3fPtr    mGeomPts;

   // The visualisation geometry, needed for update.
   OSG::GeometryNodePtr mGeomNode;
   OSG::SwitchNodePtr   mSwitchNode;

   /** @name Ray Properties */
   //@{
   float        mRayLength;     /**< The length of the rendered "ray" */
   OSG::Color4f mRayDiffuse;
   OSG::Color4f mRayAmbient;
   OSG::Real32  mRayWidth;
   //@}
};

}
#endif /*_INF_POINT_INTERSECTION_STRATEGY_H_*/
