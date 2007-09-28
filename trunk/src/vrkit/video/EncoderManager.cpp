// vrkit is (C) Copyright 2005-2007
//    by Allen Bierbaum, Aron Bierbuam, Patrick Hartling, and Daniel Shipton
//
// This file is part of vrkit.
//
// vrkit is free software; you can redistribute it and/or modify it under the
// terms of the GNU Lesser General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// vrkit is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
// more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <sstream>

#include <vpr/Util/Assert.h>

#include <vrkit/Status.h>
#include <vrkit/Exception.h>
#include <vrkit/video/Encoder.h>
#include <vrkit/video/EncoderManager.h>

#ifndef GL_COLOR_ATTACHMENT0_EXT
#  define GL_COLOR_ATTACHMENT0_EXT 0x8CE0
#endif

#ifdef VRKIT_WITH_FFMPEG
#  include <vrkit/video/EncoderFFmpeg.h>
#endif

#if defined(VRKIT_WITH_VFW)
#  include <vrkit/video/EncoderVFW.h>
#endif

#if defined(VRKIT_WITH_DIRECT_SHOW)
#  include <vrkit/video/EncoderDirectShow.h>
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

namespace vrkit
{

namespace video
{

EncoderManager::EncoderManager()
   : mRecording(false)
   , mPaused(false)
   , mStereo(false)
   , mFilename("vrkit_movie.avi")
   , mFps(30)
   , mWidth(512)
   , mHeight(512)
   , mEncoder()
{
   /* Do nothing. */ ;
}

EncoderManagerPtr EncoderManager::create()
{
   return EncoderManagerPtr(new EncoderManager);
}

EncoderManager::~EncoderManager()
{
   if (NULL != mEncoder.get())
   {
      mEncoder->stopEncoding();
      mEncoder = EncoderPtr();
   }
}

EncoderManagerPtr EncoderManager::init()
{
#ifdef VRKIT_WITH_FFMPEG
   try
   {
      REGISTER_ENCODER(EncoderFFmpeg)
   }
   catch (Exception& ex)
   {
      VRKIT_STATUS << "Failed to register FFmpeg encoder:\n" << ex.what()
                   << std::endl;
   }
#endif

#ifdef VRKIT_WITH_VFW
   try
   {
      REGISTER_ENCODER(EncoderVFW)
   }
   catch (Exception& ex)
   {
      VRKIT_STATUS << "Failed to register VFW encoder:\n" << ex.what()
                   << std::endl;
   }
#endif

#ifdef VRKIT_WITH_DIRECT_SHOW
   try
   {
      REGISTER_ENCODER(EncoderDirectShow)
   }
   catch (Exception& ex)
   {
      VRKIT_STATUS << "Failed to register DirectShow encoder:\n" << ex.what()
                   << std::endl;
   }
#endif

   return shared_from_this();
}

bool EncoderManager::record()
{
   const OSG::UInt32 image_width(mStereo ? mWidth * 2 : mWidth);
   const OSG::UInt32 image_height(mHeight);

#if 0
   codec_map_t::const_iterator found = mCodecMap.find(mCodec);
   if (mCodecMap.end() == found)
   {
      std::stringstream ss;
      ss << "Can't find encoder for codec: " << mCodec;
      throw Exception(ss.str(), VRKIT_LOCATION);
   }
#endif

   if (NULL != mEncoder.get())
   {
      mEncoder->stopEncoding();
      mEncoder = EncoderPtr();
   }

   if ( mEncoderMap.count(mVideoEncoderParams.mEncoderName) > 0 )
   {
      encoder_map_t::const_iterator vid_encoder =
         mEncoderMap.find(mVideoEncoderParams.mEncoderName);
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
      return true;
   }
   return false;
}

void EncoderManager::pause()
{
   if ( mRecording && ! mPaused )
   {
      mPaused = true;
      mEncodingPaused();
   }
}

void EncoderManager::resume()
{
   if ( mPaused )
   {
      mPaused = false;
      mEncodingResumed();
   }
}

void EncoderManager::stop()
{
   if ( mRecording )
   {
      mEncoder->stopEncoding();
      mEncoder = EncoderPtr();
      mEncodingStopped();
   }

   mRecording = false;
   mPaused    = false;
}

void EncoderManager::setFilename(const std::string& filename)
{
   mFilename = filename;
}

void EncoderManager::setFormat(const video_encoder_format_t& format)
{
   mVideoEncoderParams = format;
}

OSG::Image::PixelFormat EncoderManager::getPixelFormat() const
{
   vprASSERT(mEncoder.get() != NULL);
   return mEncoder->getPixelFormat();
}

void EncoderManager::setSize(const OSG::UInt32 width, const OSG::UInt32 height)
{
   mWidth = width;
   mHeight= height;
}

void EncoderManager::setStereo(const bool isStereo)
{
   mStereo = isStereo;
}

void EncoderManager::setFramesPerSecond(OSG::UInt32 fps)
{
   mFps = fps;
}

void EncoderManager::writeFrame(OSG::ImagePtr img)
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

}
