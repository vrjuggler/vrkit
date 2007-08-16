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
   std::string ENCODER ## name = ENCODER::getName();            \
                                                                \
   /* Register the creator. */                                  \
   mCreatorMap[ENCODER ## name] = &ENCODER::create;             \
                                                                \
   /* Register each codec type. */                              \
   ENCODER::codec_list_t ENCODER ## cl = ENCODER::getCodecs();  \
   for (Encoder::codec_list_t::const_iterator itr =             \
        ENCODER ## cl.begin();                                  \
        itr != ENCODER ## cl.end(); ++itr)                      \
   {                                                            \
      mCodecMap[*itr].push_back(ENCODER ## name);               \
      mCodecSet.insert(*itr);					\
   }

namespace inf
{

VideoEncoder::VideoEncoder()
   : mRecording(false)
   , mImage(OSG::NullFC)
   , mStereo(false)
   , mFilename("iov_movie.avi")
   , mCodec("")
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
      mEncoder->close();
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

   codec_map_t::const_iterator found = mCodecMap.find(mCodec);
   if (mCodecMap.end() == found)
   {
      std::stringstream ss;
      ss << "Can't find encoder for codec: " << mCodec;
      throw inf::Exception(ss.str(), IOV_LOCATION);
   }

   if (NULL != mEncoder.get())
   {
      mEncoder->close();
   }

   // Get the first encoder.
   std::string encoder_name = (*found).second[0];
   vprASSERT(mCreatorMap.count(encoder_name) > 0 && "Must have the encoder.");
   // Create new encoder.
   encoder_create_t creator = mCreatorMap[encoder_name];
   mEncoder = creator()->init(mFilename, mCodec, image_width, image_height, mFps);

   // Create the image to store the pixel data in.
   mImage = OSG::Image::create();
   OSG::beginEditCP(mImage);
// Video for Windows wants bytes in BRG order. Ask the GL driver for them in that order.
#if defined(IOV_WITH_VFW)
      mImage->set(OSG::Image::OSG_BGR_PF, 1);
#else
      mImage->set(OSG::Image::OSG_RGB_PF, 1);
#endif
   OSG::endEditCP(mImage);

   // Fill in the image.
   mImage->set(mImage->getPixelFormat(), image_width, image_height);

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
      mEncoder->close();
      mEncoder = EncoderPtr();
   }
   mRecording = false;
}

void VideoEncoder::setFilename(const std::string& filename)
{
   mFilename = filename;
}

void VideoEncoder::setCodec(const std::string& codec)
{
   mCodec = codec;
}

void VideoEncoder::setSize(OSG::UInt32 width, OSG::UInt32 height)
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

   mEncoder->writeFrame(mEncoder->width(), mEncoder->height(), mImage->getData());
}

}
