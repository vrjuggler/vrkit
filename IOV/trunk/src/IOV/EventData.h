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

struct EventResult
{
   typedef int result_type;
   static const result_type DONE = 0;
   static const result_type CONTINUE = 1;

   template<typename InputIterator>
   result_type operator()(InputIterator first, InputIterator last) const
   {
      while (first != last)
      {
         //result_type result = *first;
         if (DONE == *first)
         {
            return DONE;
         }
         ++first;
      }
      return DONE;
   }
};

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
   boost::signal< int (OSG::TransformNodePtr, gmtl::Matrix44f), EventResult > mObjectMovedSignal;

protected:
   EventData();
};

}

#endif /*_INF_EVENT_DATA_H_*/
