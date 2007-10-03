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

#ifndef _VRKIT_EVENT_DATA_H_
#define _VRKIT_EVENT_DATA_H_

#include <vrkit/Config.h>

#include <vector>
#include <utility>
#include <boost/signal.hpp>

#include <vpr/vpr.h>
#include <vpr/Util/GUID.h>

#include <gmtl/Matrix.h>
#include <gmtl/Point.h>

#include <vrkit/SceneData.h>
#include <vrkit/SceneObjectPtr.h>
#include <vrkit/scenedata/Event.h>
#include <vrkit/scenedata/EventDataPtr.h>


namespace vrkit
{

/** \class EventData EventData.h vrkit/scenedata/EventData.h
 *
 * Contains event signals that you can register with to recieve events
 * when they are activated.
 */
class VRKIT_CLASS_API EventData : public SceneData
{
protected:
   EventData();

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

   /**
    * The type for signals whose slots take a single argument of type
    * vrkit::SceneObjectPtr.
    */
   typedef boost::signal<event::ResultType (SceneObjectPtr),
                         event::ResultOperator> basic_action_t;
   typedef boost::signal<event::ResultType (const std::vector<SceneObjectPtr>&),
                         event::ResultOperator> multi_obj_action_t;

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
   typedef boost::signal<event::ResultType (const moved_obj_list_t&),
                         event::ResultOperator>
      obj_move_action_t;
   //@}

   /** @name Signals */
   //@{
   /**
    * Signal emitted when one or more objects are moved.
    *
    * @since 0.45.0
    */
   obj_move_action_t objectsMoved;

   typedef boost::signal<
      event::ResultType(SceneObjectPtr, const gmtl::Point3f&),
      event::ResultOperator
   > obj_isect_action_t;

   /**
    * Signal emitted when an object is intersected. This is similar to a
    * mouse-over or mouse-enter event in traditional 2D windowing systems.
    *
    * @since 0.45.0
    */
   obj_isect_action_t objectIntersected;

   /**
    * Signal emitted when an object changes from being intersected to not being
    * intersected. This is similar to a mouse-exit event in traditional 2D
    * windowing systems.
    *
    * @since 0.45.0
    */
   basic_action_t objectDeintersected;

   /**
    * Signal emitted when one or more objects are selected/grabbed.
    *
    * @since 0.45.0
    */
   multi_obj_action_t objectsSelected;

   /**
    * Signal emitted when one or more selected/grabed objects are
    * de-selected/released.
    *
    * @since 0.45.0
    */
   multi_obj_action_t objectsDeselected;

   /**
    * Signal emitted when one or more objects are chosen to be grabbed later.
    * This may be emitted multiple times before mObjectsSelectedSignal is
    * emitted.
    *
    * @since 0.45.0
    */
   multi_obj_action_t selectionListExpanded;

   /**
    * Signal emitted when one or more objects are removed from the collection
    * of objects chosen to be grabbed later. This may be emitted multiple
    * times before mObjectsSelectedSignal is emitted.
    *
    * @since 0.45.0
    */
   multi_obj_action_t selectionListReduced;

   /**
    * Signal emitted when an object is picked. Picking is different than
    * selecting.
    *
    * @since 0.45.0
    */
   basic_action_t objectPicked;

   /**
    * Signal emitted when a picked object is "unpicked."
    *
    * @since 0.45.0
    */
   basic_action_t objectUnpicked;
   //@}
};

}


#endif /* _VRKIT_EVENT_DATA_H_ */
