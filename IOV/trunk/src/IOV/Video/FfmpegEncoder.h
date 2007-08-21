// Copyright (C) Infiscape Corporation 2005-2007

#ifdef IOV_WITH_FFMPEG

#ifndef _INF_FFMPEG_ENCODER_H_
#define _INF_FFMPEG_ENCODER_H_

#include <IOV/Config.h>

#include <string>
#include <exception>
#include <boost/enable_shared_from_this.hpp>
#include <vpr/vprTypes.h>

#include <IOV/Video/Encoder.h>


struct AVFrame;
struct AVCodec;
struct AVFormatContext;
struct AVOutputFormat;
struct AVStream;

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
   FfmpegEncoder();

public:
   static EncoderPtr create();

   virtual ~FfmpegEncoder();

   /**
    * Initialize the video grabber.
    *
    */
   EncoderPtr init();

   static std::string getName()
   {
      return "FFmpegEncoder";
   }

   /**
    * Open the video stream and start encoding.
    */
   virtual void startEncoding();

   /**
    * Close the video stream.
    */
   virtual void stopEncoding();

   /**
    * Encode a frame using the current RGB data.
    */
   void writeFrame(vpr::Uint8* data);

private:
   /**
    * Allocate a new movie frame.
    */
   AVFrame* allocFrame(int pixFormat, int width, int height);

   /**
    * Add a video stream to the format context.
    */
   void addVideoStream(AVCodec* codec);

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
   unsigned char*    mVideoOutBuffer;
   unsigned int      mVideoOutBufferSize;
   unsigned int      mFrameCount;

   bool              mFlipBeforeEncode;
};

}

#endif /*_INF_FFMPEG_ENCODER_H_*/

#endif /*IOV_WITH_FFMPEG*/
