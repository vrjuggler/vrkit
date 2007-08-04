// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_VIEWER_PTR_H_
#define _INF_VIEWER_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class Viewer;
   typedef boost::shared_ptr<Viewer> ViewerPtr;
   typedef boost::weak_ptr<Viewer> ViewerWeakPtr;
}

#endif
