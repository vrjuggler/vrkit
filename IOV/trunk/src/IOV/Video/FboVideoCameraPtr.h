// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_FBO_VIDEO_CAMERA_PTR_H_
#define _INF_FBO_VIDEO_CAMERA_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class FboVideoCamera;
   typedef boost::shared_ptr<FboVideoCamera> FboVideoCameraPtr;
   typedef boost::weak_ptr<FboVideoCamera> FboVideoCameraWeakPtr;
}

#endif

