// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_VIDEO_CAMERA_PTR_H_
#define _INF_VIDEO_CAMERA_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class VideoCamera;
   typedef boost::shared_ptr<VideoCamera> VideoCameraPtr;
   typedef boost::weak_ptr<VideoCamera> VideoCameraWeakPtr;
}

#endif

