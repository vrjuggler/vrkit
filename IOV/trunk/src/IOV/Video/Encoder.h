#ifndef _INF_ENCODER_H_
#define _INF_ENCODER_H_

#include <vector>

#include <boost/enable_shared_from_this.hpp>

#include <vpr/vprTypes.h>
#include <vpr/Util/Assert.h>
#include <IOV/Config.h>
#include <IOV/Video/EncoderPtr.h>

namespace inf
{

class IOV_CLASS_API Encoder : public boost::enable_shared_from_this<Encoder>
{
public:
   typedef std::vector<std::string> codec_list_t;

   virtual EncoderPtr init(const std::string& filename, const std::string& encoding,
                           const vpr::Uint32 width, const vpr::Uint32 height,
                           const vpr::Uint32 framePerSecond) = 0;
   virtual void writeFrame(int hidth, int height, vpr::Uint8* data) = 0;
   virtual vpr::Uint32 width() const = 0;
   virtual vpr::Uint32 height() const = 0;
   virtual void close()
   {;}
   //static codec_list_t getCodecs();
   //static std::string getName();
};

}

#endif /*_INF_ENCODER_H_*/
