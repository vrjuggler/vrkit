// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_BASIC_MOVE_STRATEGY_H_
#define _INF_BASIC_MOVE_STRATEGY_H_

#include <boost/enable_shared_from_this.hpp>

#include "../../MoveStrategy.h"


namespace inf
{

class BasicMoveStrategy
   : public inf::MoveStrategy
   , public boost::enable_shared_from_this<BasicMoveStrategy>
{
public:
   static std::string getId()
   {
      return "BasicMove";
   }

   static inf::MoveStrategyPtr create()
   {
      return inf::MoveStrategyPtr(new BasicMoveStrategy());
   }

   virtual ~BasicMoveStrategy()
   {
      /* Do nothing. */ ;
   }

   virtual void init(inf::ViewerPtr viewer);

   virtual void objectGrabbed(inf::ViewerPtr viewer,
                              OSG::TransformNodePtr obj,
                              gmtl::Point3f& intersectPoint,
                              const gmtl::Matrix44f& vp_M_wand);

   virtual void objectReleased(inf::ViewerPtr viewer,
                               OSG::TransformNodePtr obj);

   virtual gmtl::Matrix44f computeMove(inf::ViewerPtr viewer,
                                       OSG::TransformNodePtr obj,
                                       const gmtl::Matrix44f& vp_M_wand,
                                       gmtl::Matrix44f& curObjMat);

protected:
   BasicMoveStrategy()
      : inf::MoveStrategy()
   {
      /* Do nothing. */ ;
   }

   gmtl::Matrix44f m_wand_M_pobj;
};

}


#endif /* _INF_BASIC_MOVE_STRATEGY_H_ */
