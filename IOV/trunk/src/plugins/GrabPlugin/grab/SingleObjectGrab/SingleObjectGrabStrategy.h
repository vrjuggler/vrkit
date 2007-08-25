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
   SingleObjectGrabStrategy(const inf::plugin::Info& info);

public:
   static std::string getId()
   {
      return "SingleObjectGrab";
   }

   static GrabStrategyPtr create(const inf::plugin::Info& info)
   {
      return GrabStrategyPtr(new SingleObjectGrabStrategy(info));
   }

   virtual ~SingleObjectGrabStrategy();

   virtual GrabStrategyPtr init(ViewerPtr viewer, grab_callback_t grabCallback,
                                release_callback_t releaseCallback);

   virtual void setFocus(ViewerPtr viewer, const bool focused);

   virtual void update(ViewerPtr viewer);

   virtual std::vector<SceneObjectPtr> getGrabbedObjects();

private:
   static std::string getElementType()
   {
      return std::string("single_object_grab_strategy");
   }

   void configure(jccl::ConfigElementPtr elt);

   /**
    * Responds to object intersection signals.
    *
    * @param obj       The object associated with the intersection signal.
    * @param parentObj The top-most ancestor of \p obj.
    * @param pnt       The intersection point.
    */
   inf::Event::ResultType objectIntersected(inf::SceneObjectPtr obj,
                                            const gmtl::Point3f& pnt);

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
    * Responds to the signal emitted when the grabbable state of a scene object
    * changes. If \p obj is the object that is currently grabbed, then it is
    * released.
    *
    * @param obj The scene object whose grabbable state has changed.
    *
    * @see releaseGrabbedObject()
    */
   void grabbableObjStateChanged(inf::SceneObjectPtr obj);

   WandInterfacePtr mWandInterface;

   /** @name Grab/Release Callbacks */
   //@{
   grab_callback_t    mGrabCallback;
   release_callback_t mReleaseCallback;
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
