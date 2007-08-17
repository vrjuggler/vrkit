// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_ENCODER_H_
#define _INF_ENCODER_H_

#include <IOV/Config.h>

#include <vector>

#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>

#include <OpenSG/OSGImage.h>

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

   struct container_format_info_t
   {
      std::string		mFormatName;
      std::string 		mFormatLongName;
      std::vector<std::string>	mFileExtensions;
      codec_list_t 		mCodecList;
      std::string		mEncoderName;
   };

   struct encoder_parameters_t
   {
      std::string	mContainerFormat;
      std::string	mCodec;
      std::string	mFilename;
      vpr::Uint32	mWidth;
      vpr::Uint32	mHeight;
      vpr::Uint32	mFramesPerSecond;
   };

   typedef std::vector<container_format_info_t> container_format_list_t;

   virtual EncoderPtr init() = 0;

   std::string getName() const;

   Encoder::container_format_list_t getSupportedContainersAndCodecs() const;

   void setEncodingParameters(const encoder_parameters_t params);

   virtual void startEncoding() = 0;

   virtual void stopEncoding() = 0;

   virtual void writeFrame(vpr::Uint8* data) = 0;

   virtual OSG::Image::PixelFormat getPixelFormat() const;

protected:

   /* Helper functions into the encoder param struct.*/
   std::string	getContainerFormat() const;
   std::string	getCodec() const;
   vpr::Uint32	getWidth() const;
   vpr::Uint32	getHeight() const;
   std::string	getFilename() const;
   vpr::Uint32	getFramesPerSecond() const;

   std::string			mName;
   encoder_parameters_t		mEncoderParams;
   container_format_list_t	mContainerFormatInfoList;
};

}

#endif /*_INF_ENCODER_H_*/
