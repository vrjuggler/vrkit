// Copyright (C) Infiscape Corporation 2005-2007
#ifndef _INF_ENCODER_PTR_H_
#define _INF_ENCODER_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class Encoder;
   typedef boost::shared_ptr<Encoder> EncoderPtr;
   typedef boost::weak_ptr<Encoder> EncoderWeakPtr;
}

#endif /*_INF_FFMPEG_ENCODER_PTR_H_*/
