// Copyright (C) Infiscape Corporation 2005-2006

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

protected:
   ObjectSpaceMoveStrategy()
      : inf::MoveStrategy()
   {
      /* Do nothing. */ ;
   }

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
