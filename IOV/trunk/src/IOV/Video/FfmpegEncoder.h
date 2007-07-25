// Copyright (C) Infiscape Corporation 2005-2007

#ifdef IOV_WITH_FFMPEG

#ifndef _INF_FFMPEG_ENCODER_H_
#define _INF_FFMPEG_ENCODER_H_

#include <IOV/Config.h>

#include <boost/enable_shared_from_this.hpp>
#include <vpr/vprTypes.h>
#include <IOV/Video/FfmpegEncoderPtr.h>


// hack for ffmpeg - c++ issue
#ifndef int64_t_C
#define int64_t_C(c)     (c ## LL)
#define uint64_t_C(c)    (c ## ULL)
#endif

#include <stdint.h>
#ifndef INT64_C
#define INT64_C(c)      (c ## LL)
#define UINT64_C(c)     (c ## ULL)
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
//#include <ffmpeg/config.h>

extern "C"
{

#include <ffmpeg/avformat.h>
#include <ffmpeg/swscale.h>

}

#include <IOV/Video/Encoder.h>

namespace inf
{

/**
 * @since 0.37
 */
class IOV_CLASS_API VideoFfmpegEncoderException
   : public std::exception
{
public:
   VideoFfmpegEncoderException(const std::string& error) throw();

   ~VideoFfmpegEncoderException() throw();

   virtual const char* what(void) const throw();

private:
   std::string s;
};

class IOV_CLASS_API FfmpegEncoder : public Encoder
{
protected:
   FfmpegEncoder()
      : mFormatContext(NULL)
      , mFormatOut(NULL)
      , mVideoStream(NULL)
      , mAudioStream(NULL)
      , mYuvFrame(NULL)
      , mRgbFrame(NULL)
      , mAudioOutBuffer(NULL)
      , mAudioOutBufferSize(0)
      , mVideoOutBuffer(NULL)
      , mVideoOutBufferSize(0)
      , mFrameCount(0)
      , mFlipBeforeEncode(true)
   {;}

public:
   static EncoderPtr create();

   virtual ~FfmpegEncoder();

   /**
    * Initialize the video grabber.
    *
    * @param filename Movie file to save data to.
    */
   EncoderPtr init(const std::string& filename, const std::string& codec,
                   const vpr::Uint32 width, const vpr::Uint32 height,
                   const vpr::Uint32 framesPerSecond);
   /**
    * Close the video stream.
    */
   virtual void close();

   /**
    * Encode a frame using the current RGB data.
    */
   void writeFrame(int width, int height, vpr::Uint8* data);

   virtual vpr::Uint32 width() const
   { return mVideoStream->codec->width; }

   virtual vpr::Uint32 height() const
   { return mVideoStream->codec->height; }

   static codec_list_t getCodecs();

   static std::string getName()
   {
      return "FFmpegEncoder";
   }

private:
   /**
    * Allocate a new movie frame.
    */
   AVFrame* allocFrame(int pixFormat, int width, int height);

   /**
    * Add a video stream to the format context.
    */
   void addVideoStream(const vpr::Uint32 width, const vpr::Uint32 height,
                       const vpr::Uint32 fps, AVCodec* codec);

   /**
    * Add an audio stream to the format context.
    */
   void addAudioStream();

   /**
    * Open the video stream.
    */
   void openVideo(AVCodec* codec);

   /**
    * Open the audio stream.
    */
   void openAudio();

   /**
    * Close the video stream.
    */
   void closeVideo();

   /**
    * Close the audio stream.
    */
   void closeAudio();

private:
   AVFormatContext*  mFormatContext;
   AVOutputFormat*   mFormatOut;
   AVStream*         mVideoStream;
   AVStream*         mAudioStream;
   AVFrame*          mYuvFrame;
   AVFrame*          mRgbFrame;

   unsigned char*     mAudioOutBuffer;
   unsigned int       mAudioOutBufferSize;
   int                audio_input_frame_size;
   int16_t*           mAudioSamples;
   //float              mT;
   //float              mTincr;
   //float              mTincr2;
   unsigned char*    mVideoOutBuffer;
   unsigned int      mVideoOutBufferSize;
   unsigned int      mFrameCount;

   bool              mFlipBeforeEncode;
};

}

#endif /*_INF_FFMPEG_ENCODER_H_*/

#endif /*IOV_WITH_FFMPEG*/
