// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_SLIDE_MOVE_STRATEGY_H_
#define _INF_SLIDE_MOVE_STRATEGY_H_

#include <string>
#include <boost/enable_shared_from_this.hpp>

#include <OpenSG/OSGColor.h>

#include <jccl/Config/ConfigElementPtr.h>

#include <IOV/Util/Exceptions.h>
#include <IOV/Grab/MoveStrategy.h>


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
                              SceneObjectPtr obj,
                              const gmtl::Point3f& intersectPoint,
                              const gmtl::Matrix44f& vp_M_wand);

   virtual void objectReleased(inf::ViewerPtr viewer,
                               SceneObjectPtr obj);

   // Note: Use the curObjPos instead of obj->getMatrix()
   virtual gmtl::Matrix44f computeMove(inf::ViewerPtr viewer,
                                       SceneObjectPtr obj,
                                       const gmtl::Matrix44f& vp_M_wand,
                                       gmtl::Matrix44f& curObjPos);

protected:
   SlideMoveStrategy();

private:
   static std::string getElementType()
   {
      return "slide_move_strategy";
   }

   void configure(jccl::ConfigElementPtr cfgElt) throw (inf::Exception);

   float mTransValue;
   gmtl::Point3f mIntersectPoint;

   /** @name Slide Behavior Properties */
   //@{
   enum SlideTarget
   {
      ISECT_POINT = 0,
      WAND_CENTER
   };

   SlideTarget  mSlideTarget;   /**< */
   int          mAnalogNum;     /**< The wand interface analog index */
   float        mForwardValue;  /**< Value for foroward sliing (0 or 1) */
   //@}
};

}


#endif /* _INF_BASIC_MOVE_STRATEGY_H_ */
