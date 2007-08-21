// Copyright (C) Infiscape Corporation 2005-2007

#include <sstream>
#include <OpenSG/OSGFBOViewport.h>

#include <IOV/Util/Exceptions.h>
#include <IOV/Video/Encoder.h>
#include <IOV/Video/VideoEncoder.h>

#ifndef GL_COLOR_ATTACHMENT0_EXT
#   define GL_COLOR_ATTACHMENT0_EXT 0x8CE0
#endif

#ifdef IOV_WITH_FFMPEG
#   include <IOV/Video/FfmpegEncoder.h>
#endif

#if defined(IOV_WITH_VFW)
#   include <IOV/Video/VfwEncoder.h>
#endif

#if defined(IOV_WITH_DIRECT_SHOW)
#   include <IOV/Video/DirectShowEncoder.h>
#endif

#define REGISTER_ENCODER(ENCODER)                               \
   EncoderPtr encoder_ ## ENCODER = ENCODER::create()->init();  \
   mEncoderMap[ENCODER::getName()] = encoder_ ## ENCODER;	\
							        \
   Encoder::container_format_list_t enc_fmt_list ## ENCODER =   \
		  encoder_ ## ENCODER->getSupportedContainersAndCodecs();	\
   /* Register each container format list. */                   \
   mVideoEncoderFormatList.insert(				\
			   mVideoEncoderFormatList.end(),	\
			   enc_fmt_list ## ENCODER.begin(),     \
			   enc_fmt_list ## ENCODER.end());

namespace inf
{

VideoEncoder::VideoEncoder()
   : mRecording(false)
   , mImage(OSG::NullFC)
   , mStereo(false)
   , mFilename("iov_movie.avi")
   , mFps(30)
   , mWidth(512)
   , mHeight(512)
   , mEncoder()
{
   /* Do nothing. */ ;
}

VideoEncoderPtr VideoEncoder::create()
{
   return VideoEncoderPtr(new VideoEncoder);
}

VideoEncoder::~VideoEncoder()
{
   if (NULL != mEncoder.get())
   {
      mEncoder->stopEncoding();
      mEncoder = EncoderPtr();
   }
   mImage = OSG::NullFC;
}

VideoEncoderPtr VideoEncoder::init()
{

#ifdef IOV_WITH_FFMPEG
   REGISTER_ENCODER(FfmpegEncoder)
#endif

#ifdef IOV_WITH_VFW
   REGISTER_ENCODER(VfwEncoder)
#endif

#ifdef IOV_WITH_DIRECT_SHOW
   REGISTER_ENCODER(DirectShowEncoder)
#endif

   return shared_from_this();
}

void VideoEncoder::record()
{
   OSG::UInt32 source_width = ( mWidth / 2 ) * 2;
   OSG::UInt32 source_height = ( mHeight / 2 ) * 2;

   OSG::UInt32 image_width = source_width;
   OSG::UInt32 image_height = source_height;

   if (mStereo)
   {
      image_width *= 2;
   }

#if 0
   codec_map_t::const_iterator found = mCodecMap.find(mCodec);
   if (mCodecMap.end() == found)
   {
      std::stringstream ss;
      ss << "Can't find encoder for codec: " << mCodec;
      throw inf::Exception(ss.str(), IOV_LOCATION);
   }
#endif

   if (NULL != mEncoder.get())
   {
      mEncoder->stopEncoding();
   }

   vprASSERT(mEncoderMap.count(mVideoEncoderParams.mEncoderName) > 0 && "Must have the encoder.");

   encoder_map_t::const_iterator vid_encoder = mEncoderMap.find(mVideoEncoderParams.mEncoderName);
   mEncoder = (*vid_encoder).second;

   Encoder::encoder_parameters_t encoder_params =
      {
         mVideoEncoderParams.mContainerFormat,
         mVideoEncoderParams.mCodec,
         mFilename,
         image_width,
         image_height,
         mFps
      };

   mEncoder->setEncodingParameters(encoder_params);


   // Create the image to store the pixel data in.
   mImage = OSG::Image::create();
   OSG::beginEditCP(mImage);
// Video for Windows wants bytes in BRG order. Ask the GL driver for them in that order.

   //XXX: Do one call to mImage->set
#if defined(IOV_WITH_VFW)
      mImage->set(OSG::Image::OSG_BGR_PF, 1);
#else
      mImage->set(OSG::Image::OSG_RGB_PF, 1);
#endif
   OSG::endEditCP(mImage);

   // Fill in the image.
   mImage->set(mImage->getPixelFormat(), image_width, image_height);

   mEncoder->startEncoding();

   mRecording = true;
}

void VideoEncoder::pause()
{
   OSG_ASSERT(NULL != mEncoder.get() && "Can't pause if we aren't recording.");
   mRecording = false;
}

void VideoEncoder::resume()
{
   OSG_ASSERT(NULL != mEncoder.get() && "Can't resume if we aren't recording.");
   if (NULL != mEncoder.get())
   {
      mRecording = true;
   }
}

void VideoEncoder::stop()
{
   OSG_ASSERT(NULL != mEncoder.get() && "Can't stop if we aren't recording.");
   if (NULL != mEncoder.get())
   {
      mEncoder->stopEncoding();
      mEncoder = EncoderPtr();
   }
   mRecording = false;
}

void VideoEncoder::setFilename(const std::string& filename)
{
   mFilename = filename;
}

void VideoEncoder::setFormat(const video_encoder_format_t& format)
{
   mVideoEncoderParams = format;
}

void VideoEncoder::setSize(const OSG::UInt32 width, const OSG::UInt32 height)
{
   mWidth = width;
   mHeight= height;
}

void VideoEncoder::setStereo(bool isStereo)
{
   mStereo = isStereo;
}

void VideoEncoder::setFramesPerSecond(OSG::UInt32 fps)
{
   mFps = fps;
}

void VideoEncoder::writeFrame(OSG::ImagePtr img)
{
   if (!mRecording)
   {
      return;
   }

   OSG::beginEditCP(mImage);
      img->reformat((OSG::Image::PixelFormat)mImage->getPixelFormat(), mImage);
   OSG::endEditCP(mImage);

   mEncoder->writeFrame(mImage->getData());
}

}
