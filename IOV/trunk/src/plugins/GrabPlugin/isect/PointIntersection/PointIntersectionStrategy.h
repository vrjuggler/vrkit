// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_POINT_INTERSECTION_STRATEGY_H_
#define _INF_POINT_INTERSECTION_STRATEGY_H_

#include <string>
#include <boost/enable_shared_from_this.hpp>

#include <gmtl/Point.h>
#include <gmtl/Matrix.h>

#include <IOV/ViewerPtr.h>
#include <IOV/Util/SceneObjectTraverser.h>
#include <IOV/Grab/IntersectionStrategy.h>


namespace inf
{

class PointIntersectionStrategy
   : public IntersectionStrategy
   , public boost::enable_shared_from_this<PointIntersectionStrategy>
{
protected:
   PointIntersectionStrategy(const inf::plugin::Info& info)
      : inf::IntersectionStrategy(info)
   {;}

public:
   virtual ~PointIntersectionStrategy()
   {;}

   static std::string getId()
   {
      return "PointIntersection";
   }

   static IntersectionStrategyPtr create(const inf::plugin::Info& info)
   {
      return IntersectionStrategyPtr(new PointIntersectionStrategy(info));
   }

   virtual inf::IntersectionStrategyPtr init(ViewerPtr viewer);

   virtual SceneObjectPtr findIntersection(ViewerPtr viewer,
      const std::vector<SceneObjectPtr>& objs, gmtl::Point3f& intersectPoint);

private:
   SceneObjectTraverser::Result enter(SceneObjectPtr obj,
                                      const gmtl::Matrix44f& vp_M_wand);

   /** @name Intersection Traversal Properties */
   //@{
   SceneObjectPtr mIntersectObj;
   OSG::Pnt3f     mIntersectPoint;
   //@}
};

}
#endif /*_INF_POINT_INTERSECTION_STRATEGY_H_*/
