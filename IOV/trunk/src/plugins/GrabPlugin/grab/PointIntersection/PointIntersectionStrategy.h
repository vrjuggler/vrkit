// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_POINT_INTERSECTION_STRATEGY_H_
#define _INF_POINT_INTERSECTION_STRATEGY_H_

#include <string>
#include <boost/enable_shared_from_this.hpp>
#include <gmtl/Point.h>
#include <IOV/ViewerPtr.h>
#include <IOV/Grab/IntersectionStrategy.h>

namespace inf
{

class PointIntersectionStrategy
   : public IntersectionStrategy
   , public boost::enable_shared_from_this<PointIntersectionStrategy>
{
protected:
   PointIntersectionStrategy()
      : inf::IntersectionStrategy()
   {;}

public:
   virtual ~PointIntersectionStrategy()
   {;}

   static std::string getId()
   {
      return "PointIntersection";
   }

   static IntersectionStrategyPtr create()
   {
      return IntersectionStrategyPtr(new PointIntersectionStrategy());
   }

   virtual inf::IntersectionStrategyPtr init(ViewerPtr viewer);

   virtual SceneObjectPtr findIntersection(ViewerPtr viewer,
      const std::vector<SceneObjectPtr>& objs, gmtl::Point3f& intersectPoint);

   /**
    * Invokes the global scope delete operator.  This is required for proper
    * releasing of memory in DLLs on Windows.
    */
   void operator delete(void* p)
   {
      ::operator delete(p);
   }

protected:
   /**
    * Deletes this object.  This is an implementation of the pure virtual
    * inf::IntersectionStrategy::destroy() method.
    */
   virtual void destroy()
   {
      delete this;
   }
};

}
#endif /*_INF_POINT_INTERSECTION_STRATEGY_H_*/
