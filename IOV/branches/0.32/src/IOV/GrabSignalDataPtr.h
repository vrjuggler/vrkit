// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_GRAB_SIGNAL_DATA_PTR_H_
#define _INF_GRAB_SIGNAL_DATA_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class GrabSignalData;
   typedef boost::shared_ptr<GrabSignalData> GrabSignalDataPtr;
   typedef boost::weak_ptr<GrabSignalData> GrabSignalDataWeakPtr;   
}

#endif /*_INF_GRAB_SIGNAL_DATA_PTR_H_*/
