// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_GRABBER_PTR_H_
#define _INF_GRABBER_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class VideoEncoder;
   typedef boost::shared_ptr<VideoEncoder> VideoEncoderPtr;
   typedef boost::weak_ptr<VideoEncoder> VideoEncoderWeakPtr;
}

#endif

