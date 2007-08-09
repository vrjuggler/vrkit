// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_ENCODER_H_
#define _INF_ENCODER_H_

#include <IOV/Config.h>

#include <vector>

#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>

#include <vpr/vprTypes.h>
#include <vpr/Util/Assert.h>

#include <IOV/Video/EncoderPtr.h>

namespace inf
{

class IOV_CLASS_API Encoder
   : public boost::enable_shared_from_this<Encoder>
   , private boost::noncopyable
{
protected:
   Encoder();

public:
   virtual ~Encoder();

   typedef std::vector<std::string> codec_list_t;

   virtual EncoderPtr init(const std::string& filename, const std::string& encoding,
                           const vpr::Uint32 width, const vpr::Uint32 height,
                           const vpr::Uint32 framePerSecond) = 0;
   virtual void writeFrame(int hidth, int height, vpr::Uint8* data) = 0;
   virtual vpr::Uint32 width() const = 0;
   virtual vpr::Uint32 height() const = 0;
   virtual void close();
};

}

#endif /*_INF_ENCODER_H_*/
