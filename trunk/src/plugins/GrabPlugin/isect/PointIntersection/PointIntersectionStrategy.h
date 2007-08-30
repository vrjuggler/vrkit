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

#ifndef _VRKIT_POINT_INTERSECTION_STRATEGY_H_
#define _VRKIT_POINT_INTERSECTION_STRATEGY_H_

#include <string>
#include <boost/enable_shared_from_this.hpp>

#include <gmtl/Point.h>
#include <gmtl/Matrix.h>

#include <vrkit/ViewerPtr.h>
#include <vrkit/util/SceneObjectTraverser.h>
#include <vrkit/isect/Strategy.h>


namespace vrkit
{

class PointIntersectionStrategy
   : public isect::Strategy
   , public boost::enable_shared_from_this<PointIntersectionStrategy>
{
protected:
   PointIntersectionStrategy(const plugin::Info& info)
      : isect::Strategy(info)
   {;}

public:
   virtual ~PointIntersectionStrategy()
   {;}

   static std::string getId()
   {
      return "PointIntersection";
   }

   static isect::StrategyPtr create(const plugin::Info& info)
   {
      return isect::StrategyPtr(new PointIntersectionStrategy(info));
   }

   virtual isect::StrategyPtr init(ViewerPtr viewer);

   virtual SceneObjectPtr findIntersection(ViewerPtr viewer,
      const std::vector<SceneObjectPtr>& objs, gmtl::Point3f& intersectPoint);

private:
   util::SceneObjectTraverser::Result enter(SceneObjectPtr obj,
                                            const gmtl::Matrix44f& vp_M_wand);

   /** @name Intersection Traversal Properties */
   //@{
   SceneObjectPtr mIntersectObj;
   OSG::Pnt3f     mIntersectPoint;
   //@}
};

}


#endif /* _VRKIT_POINT_INTERSECTION_STRATEGY_H_ */
