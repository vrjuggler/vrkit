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

#ifndef _VRKIT_BASIC_MOVE_STRATEGY_H_
#define _VRKIT_BASIC_MOVE_STRATEGY_H_

#include <map>
#include <boost/enable_shared_from_this.hpp>

#include <vrkit/move/Strategy.h>


namespace vrkit
{

class BasicMoveStrategy
   : public move::Strategy
   , public boost::enable_shared_from_this<BasicMoveStrategy>
{
protected:
   BasicMoveStrategy(const plugin::Info& info)
      : move::Strategy(info)
   {
      /* Do nothing. */ ;
   }

public:
   static std::string getId()
   {
      return "BasicMove";
   }

   static move::StrategyPtr create(const plugin::Info& info)
   {
      return move::StrategyPtr(new BasicMoveStrategy(info));
   }

   virtual ~BasicMoveStrategy()
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

   virtual gmtl::Matrix44f computeMove(ViewerPtr viewer,
                                       SceneObjectPtr obj,
                                       const gmtl::Matrix44f& vp_M_wand,
                                       const gmtl::Matrix44f& curObjMat);

private:
   void objectGrabbed(SceneObjectPtr obj, const gmtl::Matrix44f& vp_M_wand);

   /**
    * Map of transformations from the wand coordinate space into the
    * coordinate space of the parent of a grabbed scene object.
    */
   std::map<SceneObjectPtr, gmtl::Matrix44f> m_wand_M_pobj_map;
};

}


#endif /* _VRKIT_BASIC_MOVE_STRATEGY_H_ */
