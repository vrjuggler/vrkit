// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_OBJECT_SPACE_MOVE_STRATEGY_H_
#define _INF_OBJECT_SPACE_MOVE_STRATEGY_H_

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
public:
   static std::string getId()
   {
      return "ObjectSpaceMove";
   }

   static inf::MoveStrategyPtr create()
   {
      return inf::MoveStrategyPtr(new ObjectSpaceMoveStrategy());
   }

   virtual ~ObjectSpaceMoveStrategy()
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

   virtual gmtl::Matrix44f computeMove(inf::ViewerPtr viewer,
                                       SceneObjectPtr obj,
                                       const gmtl::Matrix44f& vp_M_wand,
                                       gmtl::Matrix44f& curObjPos);

protected:
   ObjectSpaceMoveStrategy()
      : inf::MoveStrategy()
   {
      /* Do nothing. */ ;
   }

   gmtl::Quatf     mRotation;
   gmtl::Matrix44f m_wand_M_vp;
   gmtl::Matrix44f m_wand_M_pobj;
   gmtl::Matrix44f m_pobj_M_obj;
   gmtl::Matrix44f m_obj_M_pobj;
};

}


#endif /* _INF_BASIC_MOVE_STRATEGY_H_ */
