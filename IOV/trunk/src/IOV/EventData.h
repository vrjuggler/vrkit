// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_EVENT_DATA_H_
#define _INF_EVENT_DATA_H_

#include <IOV/Config.h>

#include <vector>
#include <utility>
#include <boost/signal.hpp>

#include <IOV/SceneData.h>
#include <IOV/EventDataPtr.h>
#include <IOV/SceneObjectPtr.h>
#include <IOV/Event.h>

#include <vpr/vpr.h>
#include <vpr/Util/GUID.h>

#include <gmtl/Matrix.h>
#include <gmtl/Point.h>


namespace inf
{

/**
 * Contains event signals that you can register with to recieve events
 * when they are activated.
 */
class IOV_CLASS_API EventData : public inf::SceneData
{
public:
   /**
    * The unique type identifier for scene data used by plug-ins that manage
    * grabbing of objects in the scene.
    */
   static const vpr::GUID type_guid;

   static EventDataPtr create()
   {
      return EventDataPtr(new EventData);
   }

   virtual ~EventData();

public:
   /**
    * The type for signals whose slots take a single argument of type
    * inf::SceneObjectPtr.
    */
   typedef boost::signal<Event::ResultType (SceneObjectPtr),
                         Event::ResultOperator> basic_action_t;
   typedef boost::signal<Event::ResultType (const std::vector<SceneObjectPtr>&),
                         Event::ResultOperator> multi_obj_action_t;

   /** @name Object Movement Data Types */
   //@{
   /**
    * Type information for a scene object that is moved to some new
    * location described by the associated transformation matrix.
    *
    * @since 0.29.0
    */
   typedef std::pair<SceneObjectPtr, gmtl::Matrix44f> obj_move_data_t;

   typedef std::vector<obj_move_data_t> moved_obj_list_t;

   /**
    * The type of the signal emitted when one or more objects are moved.
    *
    * @since 0.29.0
    */
   typedef boost::signal<Event::ResultType (const moved_obj_list_t&),
                         Event::ResultOperator>
      obj_move_action_t;
   //@}

   /** @name Signals */
   //@{
   /**
    *
    * @since 0.29.0
    */
   obj_move_action_t mObjectsMovedSignal;

   /**
    * Signal emitted when an object is intersected. This is similar to a
    * mouse-over or mouse-enter event in traditional 2D windowing systems.
    *
    * @since 0.19.0
    */
   boost::signal<Event::ResultType (SceneObjectPtr, const gmtl::Point3f&),
                 Event::ResultOperator> mObjectIntersectedSignal;

   /**
    * Signal emitted when an object changes from being intersected to not being
    * intersected. This is similar to a mouse-exit event in traditional 2D
    * windowing systems.
    *
    * @since 0.19.0
    */
   basic_action_t mObjectDeintersectedSignal;

   /**
    * Signal emitted when one or more objects are selected/grabbed.
    *
    * @since 0.29.0
    */
   multi_obj_action_t mObjectsSelectedSignal;

   /**
    * Signal emitted when one or more selected/grabed objects are
    * de-selected/released.
    *
    * @since 0.29.0
    */
   multi_obj_action_t mObjectsDeselectedSignal;

   /**
    * Signal emitted when one or more objects are chosen to be grabbed later.
    * This may be emitted multiple times before mObjectsSelectedSignal is
    * emitted.
    *
    * @since 0.30.4
    */
   multi_obj_action_t mSelectionListExpandedSignal;

   /**
    * Signal emitted when one or more objects are removed from the collection
    * of objects chosen to be grabbed later. This may be emitted multiple
    * times before mObjectsSelectedSignal is emitted.
    *
    * @since 0.30.4
    */
   multi_obj_action_t mSelectionListReducedSignal;

   basic_action_t mObjectPickedSignal;
   basic_action_t mObjectUnpickedSignal;
   //@}

protected:
   EventData();
};

}

#endif /*_INF_EVENT_DATA_H_*/
