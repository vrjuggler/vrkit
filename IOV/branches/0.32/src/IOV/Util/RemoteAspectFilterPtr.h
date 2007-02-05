// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_REMOTE_ASPECT_FILTER_PTR_H_
#define _INF_REMOTE_ASPECT_FILTER_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class RemoteAspectFilter;
   typedef boost::shared_ptr<RemoteAspectFilter> RemoteAspectFilterPtr;
   typedef boost::weak_ptr<RemoteAspectFilter> RemoteAspectFilterWeakPtr;   
}

#endif
