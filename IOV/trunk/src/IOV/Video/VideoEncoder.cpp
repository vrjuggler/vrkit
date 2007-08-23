// Copyright (C) Infiscape Corporation 2005-2007

#include <sstream>

#include <vpr/Util/Assert.h>

#include <IOV/Status.h>
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

#define REGISTER_ENCODER(ENCODER)                                       \
   EncoderPtr encoder_ ## ENCODER = ENCODER::create()->init();          \
   mEncoderMap[ENCODER::getName()] = encoder_ ## ENCODER;               \
                                                                        \
   Encoder::container_format_list_t enc_fmt_list ## ENCODER =           \
      encoder_ ## ENCODER->getSupportedContainersAndCodecs();           \
   /* Register each container format list. */                           \
   mVideoEncoderFormatList.insert(mVideoEncoderFormatList.end(),        \
                                  enc_fmt_list ## ENCODER.begin(),      \
                                  enc_fmt_list ## ENCODER.end());

namespace inf
{

VideoEncoder::VideoEncoder()
   : mRecording(false)
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
}

VideoEncoderPtr VideoEncoder::init()
{
#ifdef IOV_WITH_FFMPEG
   try
   {
      REGISTER_ENCODER(FfmpegEncoder)
   }
   catch (inf::Exception& ex)
   {
      IOV_STATUS << "Failed to register FFmpeg encoder:\n" << ex.what()
                 << std::endl;
   }
#endif

#ifdef IOV_WITH_VFW
   try
   {
      REGISTER_ENCODER(VfwEncoder)
   }
   catch (inf::Exception& ex)
   {
      IOV_STATUS << "Failed to register VFW encoder:\n" << ex.what()
                 << std::endl;
   }
#endif

#ifdef IOV_WITH_DIRECT_SHOW
   try
   {
      REGISTER_ENCODER(DirectShowEncoder)
   }
   catch (inf::Exception& ex)
   {
      IOV_STATUS << "Failed to register DirectShow encoder:\n" << ex.what()
                 << std::endl;
   }
#endif

   return shared_from_this();
}

void VideoEncoder::record()
{
   const OSG::UInt32 image_width(mStereo ? mWidth * 2 : mWidth);
   const OSG::UInt32 image_height(mHeight);

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
   mEncoder->startEncoding();

   mRecording = true;
   mEncodingStarted();
}

void VideoEncoder::pause()
{
   vprASSERT(NULL != mEncoder.get() && "Can't pause without an encoder.");
   mRecording = false;
   mEncodingPaused();
}

void VideoEncoder::resume()
{
   vprASSERT(NULL != mEncoder.get() && "Can't resume without an encoder.");
   if (NULL != mEncoder.get())
   {
      mRecording = true;
      mEncodingResumed();
   }
}

void VideoEncoder::stop()
{
   vprASSERT(NULL != mEncoder.get() && "Can't stop without an encoder.");
   if (NULL != mEncoder.get())
   {
      mEncoder->stopEncoding();
      mEncoder = EncoderPtr();
      mEncodingStopped();
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

OSG::Image::PixelFormat VideoEncoder::getPixelFormat() const
{
   vprASSERT(mEncoder.get() != NULL);
   return mEncoder->getPixelFormat();
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

   try
   {
      mEncoder->writeFrame(img->getData());
   }
   catch (std::exception& ex)
   {
      std::cerr << "Encoder failed to write frame; stopping encoding\n"
                << ex.what() << std::endl;
      stop();
   }
}

}
