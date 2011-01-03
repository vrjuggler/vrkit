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

#ifndef _VRKIT_MULTI_OBJECT_GRAB_STRATEGY_H_
#define _VRKIT_MULTI_OBJECT_GRAB_STRATEGY_H_

#include <string>
#include <vector>
#include <boost/enable_shared_from_this.hpp>
#include <boost/signals/connection.hpp>

#include <gmtl/Matrix.h>
#include <gmtl/Point.h>

#include <jccl/Config/ConfigElementPtr.h>

#include <vrkit/SceneObjectPtr.h>
#include <vrkit/scenedata/EventDataPtr.h>
#include <vrkit/grab/Strategy.h>
#include <vrkit/util/DigitalCommand.h>


namespace vrkit
{

class MultiObjectGrabStrategy
   : public grab::Strategy
   , public boost::enable_shared_from_this<MultiObjectGrabStrategy>
{
protected:
   MultiObjectGrabStrategy(const plugin::Info& info);

public:
   static std::string getId()
   {
      return "MultiObjectGrab";
   }

   static grab::StrategyPtr create(const plugin::Info& info)
   {
      return grab::StrategyPtr(new MultiObjectGrabStrategy(info));
   }

   virtual ~MultiObjectGrabStrategy();

   virtual grab::StrategyPtr init(ViewerPtr viewer,
                                  grab_callback_t grabCallback,
                                  release_callback_t releaseCallback);

   virtual void setFocus(ViewerPtr viewer, const bool focused);

   virtual void update(ViewerPtr viewer);

   virtual std::vector<SceneObjectPtr> getGrabbedObjects();

private:
   static std::string getElementType()
   {
      return std::string("multi_object_grab_strategy");
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
    * Responds to the signal emitted when the grabbable state of a scene
    * object changes. If \p obj is no longer grabbable, we may have to update
    * our state to reflect that fact.
    *
    * @post If \p obj is in \c mChosenObjects or \c mGrabbedObjects, then it
    *       is removed. Either way, this slot is disconnected from the
    *       object's grabbable state change signal no that we are no longer
    *       interested in it.
    *
    * @param obj The scene object that was removed from vrkit::GrabData.
    */
   void grabbableObjStateChanged(SceneObjectPtr obj);

   WandInterfacePtr mWandInterface;

   EventDataPtr mEventData;

   /** @name Grab/Release Callbacks */
   //@{
   grab_callback_t    mGrabCallback;
   release_callback_t mReleaseCallback;
   //@}

   /** @name Button(s) for adding objects to selection. */
   //@{
   util::DigitalCommand mChooseBtn;
   std::string mChooseText;
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


#endif /* _VRKIT_MULTI_OBJECT_GRAB_STRATEGY_H_ */
