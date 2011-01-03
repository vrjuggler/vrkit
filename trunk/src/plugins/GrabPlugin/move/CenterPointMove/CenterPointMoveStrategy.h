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

#ifndef _VRKIT_CENTER_POINT_MOVE_STRATEGY_H_
#define _VRKIT_CENTER_POINT_MOVE_STRATEGY_H_

#include <boost/enable_shared_from_this.hpp>

#include <vrkit/move/Strategy.h>


namespace vrkit
{

class CenterPointMoveStrategy
   : public move::Strategy
   , public boost::enable_shared_from_this<CenterPointMoveStrategy>
{
protected:
   CenterPointMoveStrategy(const plugin::Info& info)
      : move::Strategy(info)
   {
      /* Do nothing. */ ;
   }

public:
   static std::string getId()
   {
      return "CenterPointMove";
   }

   static move::StrategyPtr create(const plugin::Info& info)
   {
      return move::StrategyPtr(new CenterPointMoveStrategy(info));
   }

   virtual ~CenterPointMoveStrategy()
   {
      /* Do nothing. */ ;
   }

   virtual move::StrategyPtr init(ViewerPtr viewer);

   virtual void objectsGrabbed(ViewerPtr viewer,
                               const std::vector<SceneObjectPtr>& objs,
                               const gmtl::Point3f& intersectPoint,
                               const gmtl::Matrix44f& vp_M_wand);

   virtual void objectsReleased(ViewerPtr viewer,
                                const std::vector<SceneObjectPtr>& objs);

   virtual gmtl::Matrix44f computeMove(ViewerPtr viewer, SceneObjectPtr obj,
                                       const gmtl::Matrix44f& vp_M_wand,
                                       const gmtl::Matrix44f& curObjMat);
};

}


#endif /* _VRKIT_CENTER_POINT_MOVE_STRATEGY_H_ */
