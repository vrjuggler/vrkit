// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_EVENT_DATA_H_
#define _INF_EVENT_DATA_H_

#include <IOV/Config.h>

#include <vpr/vpr.h>
#include <vpr/Util/GUID.h>

#include <IOV/SceneData.h>
#include <IOV/EventDataPtr.h>

#include <boost/signal.hpp>

#include <OpenSG/OSGTransform.h>

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
   boost::signal<void (OSG::TransformNodePtr, gmtl::Matrix44f)> mObjectMovedSignal;

protected:
   EventData();
};

}

#endif /*_INF_EVENT_DATA_H_*/
