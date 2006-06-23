// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_SINGLE_OBJECT_GRAB_STRATEGY_H_
#define _INF_SINGLE_OBJECT_GRAB_STRATEGY_H_

#include <string>
#include <boost/enable_shared_from_this.hpp>
#include <boost/signals/connection.hpp>

#include <gmtl/Matrix.h>
#include <gmtl/Point.h>

#include <jccl/Config/ConfigElementPtr.h>

#include <IOV/SceneObjectPtr.h>
#include <IOV/Grab/GrabStrategy.h>
#include <IOV/Util/DigitalCommand.h>


namespace inf
{

class SingleObjectGrabStrategy
   : public GrabStrategy
   , public boost::enable_shared_from_this<SingleObjectGrabStrategy>
{
protected:
   SingleObjectGrabStrategy();

public:
   static std::string getId()
   {
      return "SingleObjectGrab";
   }

   static GrabStrategyPtr create()
   {
      return GrabStrategyPtr(new SingleObjectGrabStrategy());
   }

   virtual ~SingleObjectGrabStrategy();

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
      return std::string("single_object_grab_strategy");
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
    * @post \c mGrabbing is false, and \c mGrabbedObj holds a NULL pointer.
    */
   void releaseGrabbedObject();

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

   WandInterfacePtr mWandInterface;

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
   bool mIntersecting;
   SceneObjectPtr mIntersectedObj;
   gmtl::Point3f mIntersectPoint;
   boost::signals::connection mIsectConnection;
   boost::signals::connection mDeIsectConnection;
   //@}

   /** @name Grab State */
   //@{
   bool mGrabbing;
   SceneObjectPtr mGrabbedObj;
   boost::signals::connection mGrabbedObjConnection;
   //@}
};

}


#endif /* _INF_SINGLE_OBJECT_GRAB_STRATEGY_H_ */
