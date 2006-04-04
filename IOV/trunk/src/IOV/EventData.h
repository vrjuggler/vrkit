// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_EVENT_DATA_H_
#define _INF_EVENT_DATA_H_

#include <IOV/Config.h>

#include <vpr/vpr.h>
#include <vpr/Util/GUID.h>

#include <IOV/SceneData.h>
#include <IOV/EventDataPtr.h>
#include <IOV/SceneObjectPtr.h>
#include <IOV/Widget/Event.h>

#include <boost/signal.hpp>

#include <gmtl/Matrix.h>

namespace inf
{

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

   /** @name Signals */
   //@{
   boost::signal< Event::ResultType (SceneObjectPtr, gmtl::Matrix44f), Event::ResultOperator > mObjectMovedSignal;

   /**
    * Signal emitted when an object is intersected. This is similar to a
    * mouse-over or mouse-enter event in traditional 2D windowing systems.
    *
    * @since 0.19.0
    */
   basic_action_t mObjectIntersectedSignal;

   /**
    * Signal emitted when an object changes from being intersected to not being
    * intersected. This is similar to a mouse-exit event in traditional 2D
    * windowing systems.
    *
    * @since 0.19.0
    */
   basic_action_t mObjectDeintersectedSignal;

   basic_action_t mObjectSelectedSignal;
   basic_action_t mObjectDeselectedSignal;
   //@}

protected:
   EventData();
};

}

#endif /*_INF_EVENT_DATA_H_*/
