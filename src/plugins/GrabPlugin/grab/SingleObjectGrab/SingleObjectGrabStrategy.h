// vrkit is (C) Copyright 2005-2011
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

#ifndef _VRKIT_SINGLE_OBJECT_GRAB_STRATEGY_H_
#define _VRKIT_SINGLE_OBJECT_GRAB_STRATEGY_H_

#include <string>
#include <boost/enable_shared_from_this.hpp>
#include <boost/signals/connection.hpp>

#include <gmtl/Matrix.h>
#include <gmtl/Point.h>

#include <jccl/Config/ConfigElementPtr.h>

#include <vrkit/SceneObjectPtr.h>
#include <vrkit/grab/Strategy.h>
#include <vrkit/util/DigitalCommand.h>


namespace vrkit
{

class SingleObjectGrabStrategy
   : public grab::Strategy
   , public boost::enable_shared_from_this<SingleObjectGrabStrategy>
{
protected:
   SingleObjectGrabStrategy(const plugin::Info& info);

public:
   static std::string getId()
   {
      return "SingleObjectGrab";
   }

   static grab::StrategyPtr create(const plugin::Info& info)
   {
      return grab::StrategyPtr(new SingleObjectGrabStrategy(info));
   }

   virtual ~SingleObjectGrabStrategy();

   virtual grab::StrategyPtr init(ViewerPtr viewer,
                                  grab_callback_t grabCallback,
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
   event::ResultType objectIntersected(SceneObjectPtr obj,
                                       const gmtl::Point3f& pnt);

   /**
    * Responds to object de-intersection signals.
    *
    * @param obj The object associated with the de-intersection signal.
    */
   event::ResultType objectDeintersected(SceneObjectPtr obj);

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
   void grabbableObjStateChanged(SceneObjectPtr obj);

   WandInterfacePtr mWandInterface;

   /** @name Grab/Release Callbacks */
   //@{
   grab_callback_t    mGrabCallback;
   release_callback_t mReleaseCallback;
   //@}

   /** @name Button(s) for grabbing objects. */
   //@{
   util::DigitalCommand mGrabBtn;
   std::string mGrabText;
   //@}

   /** @name Button(s) for releasing objects. */
   //@{
   util::DigitalCommand mReleaseBtn;
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


#endif /* _VRKIT_SINGLE_OBJECT_GRAB_STRATEGY_H_ */
