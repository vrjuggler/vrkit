// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_MULTI_OBJECT_GRAB_STRATEGY_H_
#define _INF_MULTI_OBJECT_GRAB_STRATEGY_H_

#include <string>
#include <vector>
#include <boost/enable_shared_from_this.hpp>
#include <boost/signals/connection.hpp>

#include <gmtl/Matrix.h>
#include <gmtl/Point.h>

#include <jccl/Config/ConfigElementPtr.h>

#include <IOV/EventDataPtr.h>
#include <IOV/SceneObjectPtr.h>
#include <IOV/Grab/GrabStrategy.h>
#include <IOV/Util/DigitalCommand.h>


namespace inf
{

class MultiObjectGrabStrategy
   : public GrabStrategy
   , public boost::enable_shared_from_this<MultiObjectGrabStrategy>
{
protected:
   MultiObjectGrabStrategy();

public:
   static std::string getId()
   {
      return "MultiObjectGrab";
   }

   static GrabStrategyPtr create()
   {
      return GrabStrategyPtr(new MultiObjectGrabStrategy());
   }

   virtual ~MultiObjectGrabStrategy();

   virtual GrabStrategyPtr init(ViewerPtr viewer);

   virtual void setFocus(ViewerPtr viewer, const bool focused);

   virtual void update(ViewerPtr viewer, grab_callback_t grabCallback,
                       release_callback_t releaseCallback);

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
    * inf::GrabStrategy::destroy() method.
    */
   virtual void destroy()
   {
      delete this;
   }

private:
   static std::string getElementType()
   {
      return std::string("multi_object_grab_strategy");
   }

   void configure(jccl::ConfigElementPtr elt);

   std::vector<int> transformButtonVec(const std::vector<int>& btns);

   /**
    * Responds to object intersection signals.
    *
    * @param obj       The object associated with the intersection signal.
    * @param parentObj The top-most ancestor of \p obj.
    * @param pnt       The intersection point.
    * @param viewer    The VR Juggler application object.
    */
   inf::Event::ResultType objectIntersected(inf::SceneObjectPtr obj,
                                            inf::SceneObjectPtr parentObj,
                                            const gmtl::Point3f& pnt,
                                            inf::ViewerPtr viewer);

   /**
    * Responds to object de-intersection signals.
    *
    * @param obj The object associated with the de-intersection signal.
    */
   inf::Event::ResultType objectDeintersected(inf::SceneObjectPtr obj);

   /**
    * Responds to the signal emitted when an object is removed from
    * inf::GrabData. If \p obj is the object that is currently grabbed, then
    * the grabbed object is released.
    *
    * @param obj The scene object that was removed from inf::GrabData.
    *
    * @see releaseGrabbedObject()
    */
   void grabbableObjStateChanged(inf::SceneObjectPtr obj);

   EventDataPtr mEventData;

   WandInterfacePtr mWandInterface;

   /** @name Button(s) for adding objects to selection. */
   //@{
   inf::DigitalCommand mChooseBtn;
   std::string mChooseText;
   //@}

   /** @name Button(s) for grabbing objects. */
   //@{
   inf::DigitalCommand mGrabBtn;
   std::string mGrabText;
   //@}

   /** @name Button(s) for releasing objects. */
   //@{
   inf::DigitalCommand mReleaseBtn;
   std::string mReleaseText;
   //@}

   /** @name Intersection State */
   //@{
   SceneObjectPtr mCurIsectObject;
   std::vector<SceneObjectPtr> mChosenObjects;
   gmtl::Point3f mCurIntersectPoint;
   gmtl::Point3f mIntersectPoint;
   std::vector<boost::signals::connection> mIsectConnections;
   //@}

   /** @name Grab State */
   //@{
   bool mGrabbing;
   std::vector<SceneObjectPtr> mGrabbedObjects;
   //@}

   typedef std::map<SceneObjectPtr, boost::signals::connection> obj_conn_map_t;
   obj_conn_map_t mObjConnections;
};

}


#endif /* _INF_MULTI_OBJECT_GRAB_STRATEGY_H_ */
