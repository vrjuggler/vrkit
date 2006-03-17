// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_SLIDE_MOVE_STRATEGY_H_
#define _INF_SLIDE_MOVE_STRATEGY_H_

#include <boost/enable_shared_from_this.hpp>

#include "../../MoveStrategy.h"


namespace inf
{

class SlideMoveStrategy
   : public inf::MoveStrategy
   , public boost::enable_shared_from_this<SlideMoveStrategy>
{
public:
   static std::string getId()
   {
      return "SlideMove";
   }

   static inf::MoveStrategyPtr create()
   {
      return inf::MoveStrategyPtr(new SlideMoveStrategy());
   }

   virtual ~SlideMoveStrategy()
   {
      /* Do nothing. */ ;
   }

   virtual void init(inf::ViewerPtr viewer);

   virtual void objectGrabbed(inf::ViewerPtr viewer,
                              OSG::TransformNodePtr obj,
                              const gmtl::Matrix44f& vp_M_wand);

   virtual void objectReleased(inf::ViewerPtr viewer,
                               OSG::TransformNodePtr obj);

   // Note: Use the curObjPos instead of obj->getMatrix()
   virtual gmtl::Matrix44f computeMove(inf::ViewerPtr viewer,
                                       OSG::TransformNodePtr obj,
                                       const gmtl::Matrix44f& vp_M_wand,
                                       gmtl::Matrix44f& curObjPos);

protected:
   SlideMoveStrategy()
      : inf::MoveStrategy(), mTransValue(0.0f)
   {
      /* Do nothing. */ ;
   }

   float mTransValue;
   gmtl::Matrix44f m_wand_M_obj;
};

}


#endif /* _INF_BASIC_MOVE_STRATEGY_H_ */
