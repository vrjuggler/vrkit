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

#ifdef VRKIT_WITH_FFMPEG

#ifndef _VRKIT_VIDEO_ENCODER_FFMPEG_H_
#define _VRKIT_VIDEO_ENCODER_FFMPEG_H_

#include <vrkit/Config.h>

#include <stdint.h>
#include <string>

#include <vpr/vprTypes.h>

#include <vrkit/video/Encoder.h>


struct AVFrame;
struct AVCodec;
struct AVFormatContext;
struct AVOutputFormat;
struct AVStream;

namespace vrkit
{

namespace video
{

/** \class EncoderFFmpeg EncoderFFmpeg.h vrkit/video/EncoderFFmpeg.h
 *
 * This is the video encoder implementation based on the FFmpeg libraries
 * (libavcodec, libavformat, etc.).
 *
 * @note This class was renamed from vrkit::FfmpegEncoder and moved into the
 *       vrkit::video namespace in version 0.47.
 */
class EncoderFFmpeg : public Encoder
{
protected:
   EncoderFFmpeg();

public:
   static EncoderPtr create();

   virtual ~EncoderFFmpeg();

   /**
    * Initialize the video grabber.
    *
    */
   EncoderPtr init();

   static std::string getName()
   {
      return "FFmpeg Encoder";
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
   AVFrame* allocFrame(const int pixFormat, const int width,
                       const int height);

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

   unsigned char*    mAudioOutBuffer;
   unsigned int      mAudioOutBufferSize;
   int               audio_input_frame_size;
   int16_t*          mAudioSamples;
   unsigned char*    mVideoOutBuffer;
   unsigned int      mVideoOutBufferSize;
   unsigned int      mFrameCount;

   bool              mFlipBeforeEncode;
};

}

}


#endif /* _VRKIT_VIDEO_ENCODER_FFMPEG_H_ */

#endif /* VRKIT_WITH_FFMPEG */
