// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_CENTER_POINT_MOVE_STRATEGY_H_
#define _INF_CENTER_POINT_MOVE_STRATEGY_H_

#include <boost/enable_shared_from_this.hpp>

#include <IOV/Grab/MoveStrategy.h>


namespace inf
{

class CenterPointMoveStrategy
   : public inf::MoveStrategy
   , public boost::enable_shared_from_this<CenterPointMoveStrategy>
{
public:
   static std::string getId()
   {
      return "CenterPointMove";
   }

   static inf::MoveStrategyPtr create()
   {
      return inf::MoveStrategyPtr(new CenterPointMoveStrategy());
   }

   virtual ~CenterPointMoveStrategy()
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
                                       const gmtl::Matrix44f& curObjMat);

protected:
   CenterPointMoveStrategy()
      : inf::MoveStrategy()
   {
      /* Do nothing. */ ;
   }
};

}


#endif /* _INF_CENTER_POINT_MOVE_STRATEGY_H_ */
