// Copyright (C) Infiscape Corporation 2005-2007

#ifdef IOV_WITH_FFMPEG

#ifndef _INF_FFMPEG_ENCODER_PTR_H_
#define _INF_FFMPEG_ENCODER_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class FfmpegEncoder;
   typedef boost::shared_ptr<FfmpegEncoder> FfmpegEncoderPtr;
   typedef boost::weak_ptr<FfmpegEncoder> FfmpegEncoderWeakPtr;
}

#endif

#endif /*IOV_WITH_FFMPEG*/
