// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_CAMERA_PTR_H_
#define _INF_CAMERA_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class Camera;
   typedef boost::shared_ptr<Camera> CameraPtr;
   typedef boost::weak_ptr<Camera> CameraWeakPtr;
}

#endif

