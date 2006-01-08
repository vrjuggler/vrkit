// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_GRAB_DATA_PTR_H_
#define _INF_GRAB_DATA_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class GrabData;
   typedef boost::shared_ptr<GrabData> GrabDataPtr;
   typedef boost::weak_ptr<GrabData> GrabDataWeakPtr;   
}

#endif
