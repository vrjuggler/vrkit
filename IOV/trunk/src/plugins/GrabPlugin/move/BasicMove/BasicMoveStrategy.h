// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_BASIC_MOVE_STRATEGY_H_
#define _INF_BASIC_MOVE_STRATEGY_H_

#include <map>
#include <boost/enable_shared_from_this.hpp>

#include <IOV/Grab/MoveStrategy.h>


namespace inf
{

class BasicMoveStrategy
   : public inf::MoveStrategy
   , public boost::enable_shared_from_this<BasicMoveStrategy>
{
protected:
   BasicMoveStrategy(const inf::plugin::Info& info)
      : inf::MoveStrategy(info)
   {
      /* Do nothing. */ ;
   }

public:
   static std::string getId()
   {
      return "BasicMove";
   }

   static inf::MoveStrategyPtr create(const inf::plugin::Info& info)
   {
      return inf::MoveStrategyPtr(new BasicMoveStrategy(info));
   }

   virtual ~BasicMoveStrategy()
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

   /**
    * Invokes the global scope delete operator.  This is required for proper
    * releasing of memory in DLLs on Windows.
    */
   void operator delete(void* p)
   {
      ::operator delete(p);
   }

protected:
   /**
    * Deletes this object.  This is an implementation of the pure virtual
    * inf::MoveStrategy::destroy() method.
    */
   virtual void destroy()
   {
      delete this;
   }

private:
   void objectGrabbed(SceneObjectPtr obj, const gmtl::Matrix44f& vp_M_wand);

   /**
    * Map of transformations from the wand coordinate space into the
    * coordinate space of the parent of a grabbed scene object.
    */
   std::map<SceneObjectPtr, gmtl::Matrix44f> m_wand_M_pobj_map;
};

}


#endif /* _INF_BASIC_MOVE_STRATEGY_H_ */
