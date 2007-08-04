// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_EVENT_DATA_PTR_H_
#define _INF_EVENT_DATA_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class EventData;
   typedef boost::shared_ptr<EventData> EventDataPtr;
   typedef boost::weak_ptr<EventData> EventDataWeakPtr;
}

#endif /*_INF_EVENT_DATA_PTR_H_*/
