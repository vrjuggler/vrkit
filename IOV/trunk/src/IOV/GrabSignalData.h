// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_GRAB_SIGNAL_DATA_H_
#define _INF_GRAB_SIGNAL_DATA_H_

#include <IOV/Config.h>

#include <vector>
#include <boost/signal.hpp>

#include <gmtl/Point.h>

#include <vpr/Util/GUID.h>

#include <IOV/SceneData.h>
#include <IOV/Event.h>
#include <IOV/SceneObjectPtr.h>
#include <IOV/GrabSignalDataPtr.h>


namespace inf
{

/**
 * Scene data used for centralized access to signals emitted when various
 * grab events occur. Currently, these signals are emitted by a strategy
 * plug-in (inf::SignalGrabStrategy) used by the Grab Plug-in.
 *
 * @since 0.30.3
 */
class IOV_CLASS_API GrabSignalData
   : public inf::SceneData
{
protected:
   GrabSignalData();

public:
   /**
    * The unique type identifier for scene data used by plug-ins that manage
    * grabbing of objects in the scene.
    */
   static const vpr::GUID type_guid;

   static GrabSignalDataPtr create()
   {
      return GrabSignalDataPtr(new GrabSignalData());
   }

   virtual ~GrabSignalData();

   /** @name Signals */
   //@{
   /** Signal type for choosing objects to grab. */
   typedef boost::signal<Event::ResultType (), Event::ResultOperator>
      choose_action_t;

   /** Signal type for grabbing chosen objects. */
   typedef boost::signal<
      Event::ResultType (std::vector<SceneObjectPtr>&, gmtl::Point3f&),
      Event::ResultOperator
   > grab_action_t;

   /** Signal type for releasing grabbed objects. */
   typedef boost::signal<
      Event::ResultType (std::vector<SceneObjectPtr>&), Event::ResultOperator
   > release_action_t;

   /**
    * Signal emitted when the user chooses an object to add to the
    * collection of objects to be grabbed later.
    */
   choose_action_t  choose;

   /** Signal emitted when the user grabs one or more objects. */
   grab_action_t grab;

   /** Signal emitted when the user releases grabbed objects. */
   release_action_t release;
   //@}
};

}


#endif /* _INF_GRAB_SIGNAL_DATA_H_ */
