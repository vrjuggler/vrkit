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

#include <ffmpeg/avformat.h>
#include <ffmpeg/swscale.h>

namespace inf
{

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

class IOV_CLASS_API FfmpegEncoder : public boost::enable_shared_from_this<FfmpegEncoder>
{
protected:
   FfmpegEncoder()
      : mEnabled(true)
      , mFilename()
      , mFormatContext(NULL)
      , mFormatOut(NULL)
      , mYuvFrame(NULL)
      , mRgbFrame(NULL)
      , mVideoOutBuffer(NULL)
      , mVideoOutBufferSize(0)
      , mFrameCount(0)
      , mVideoPts(0.0)
      , mFlipBeforeEncode(true)
   {;}

public:
   static FfmpegEncoderPtr create();

   virtual ~FfmpegEncoder();

   /**
    * Initialize the video grabber.
    *
    * @param filename Movie file to save data to.
    */
   FfmpegEncoderPtr init(const std::string& filename, const vpr::Uint32 width, const vpr::Uint32 height);

   void close();

private:
   bool                 mEnabled;
   std::string          mFilename;

public:
   int width() const
   { return mVideoStream->codec->width; }

   int height() const
   { return mVideoStream->codec->height; }

   unsigned char* rgb() const
   { return mRgbFrame->data[0]; }

   AVCodecContext*   context() const
   { return mVideoStream->codec; }

   unsigned int      codecid() const
   { return mVideoStream->codec->codec_id; }

   //unsigned int      fps()     const
   //{ return mVideoStream->codec->frame_rate; }

   bool              flip() const
   { return mFlipBeforeEncode; }

   unsigned int      bitrate() const
   { return mVideoStream->codec->bit_rate; }
   
   void setRgb(unsigned char* rgb);
   void setBitrate(int bitrate);
   void writeFrame();

private:

   AVFrame* allocFrame(int pix_fmt, int width, int height);

   void addVideoStream(int width, int height);
   void addAudioStream();
   void openVideo();
   void openAudio();
   void closeVideo();
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
   float              mT;
   float              mTincr;
   float              mTincr2;
   unsigned char*    mVideoOutBuffer;
   unsigned int      mVideoOutBufferSize;
   unsigned int      mFrameCount;
   double            mVideoPts;

   bool              mFlipBeforeEncode;
};

}

#endif /*_INF_FFMPEG_ENCODER_H_*/

#endif /*IOV_WITH_FFMPEG*/
