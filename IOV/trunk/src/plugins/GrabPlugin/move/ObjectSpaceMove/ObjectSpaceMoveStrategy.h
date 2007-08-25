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

#ifndef _INF_OBJECT_SPACE_MOVE_STRATEGY_H_
#define _INF_OBJECT_SPACE_MOVE_STRATEGY_H_

#include <map>
#include <boost/enable_shared_from_this.hpp>

#include <IOV/Grab/MoveStrategy.h>

#include <gmtl/Quat.h>
#include <gmtl/QuatOps.h>


namespace inf
{

class ObjectSpaceMoveStrategy
   : public inf::MoveStrategy
   , public boost::enable_shared_from_this<ObjectSpaceMoveStrategy>
{
protected:
   ObjectSpaceMoveStrategy(const inf::plugin::Info& info)
      : inf::MoveStrategy(info)
   {
      /* Do nothing. */ ;
   }

public:
   static std::string getId()
   {
      return "ObjectSpaceMove";
   }

   static inf::MoveStrategyPtr create(const inf::plugin::Info& info)
   {
      return inf::MoveStrategyPtr(new ObjectSpaceMoveStrategy(info));
   }

   virtual ~ObjectSpaceMoveStrategy()
   {
      /* Do nothing. */ ;
   }

   virtual inf::MoveStrategyPtr init(inf::ViewerPtr viewer);

   virtual void objectsGrabbed(inf::ViewerPtr viewer,
                               const std::vector<SceneObjectPtr>& objs,
                               const gmtl::Point3f& intersectPoint,
                               const gmtl::Matrix44f& vp_M_wand);

   virtual void objectsReleased(inf::ViewerPtr viewer,
                                const std::vector<SceneObjectPtr>& objs);

   virtual gmtl::Matrix44f computeMove(inf::ViewerPtr viewer,
                                       SceneObjectPtr obj,
                                       const gmtl::Matrix44f& vp_M_wand,
                                       const gmtl::Matrix44f& curObjPos);

private:
   void objectGrabbed(SceneObjectPtr obj, const gmtl::Matrix44f& vp_M_wand);

   gmtl::Quatf     mRotation;
   gmtl::Matrix44f m_wand_M_vp;

   struct ObjectData
   {
      gmtl::Matrix44f m_wand_M_pobj;
      gmtl::Matrix44f m_pobj_M_obj;
      gmtl::Matrix44f m_obj_M_pobj;
   };

   std::map<SceneObjectPtr, ObjectData> mObjectDataMap;
};

}


#endif /* _INF_BASIC_MOVE_STRATEGY_H_ */
