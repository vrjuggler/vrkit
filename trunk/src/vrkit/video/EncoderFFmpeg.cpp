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

// hack for ffmpeg - c++ issue
#ifndef int64_t_C
#  define int64_t_C(c)     (c ## LL)
#  define uint64_t_C(c)    (c ## ULL)
#endif

#ifndef INT64_C
#  define INT64_C(c)      (c ## LL)
#  define UINT64_C(c)     (c ## ULL)
#endif

#include <string>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <boost/algorithm/string.hpp>

extern "C"
{

#include <ffmpeg/avformat.h>
#include <ffmpeg/avcodec.h>
#include <ffmpeg/swscale.h>

}

#include <vrkit/Status.h>
#include <vrkit/Exception.h>
#include <vrkit/exceptions/RecordingException.h>
#include <vrkit/exceptions/RecordingConfigError.h>
#include <vrkit/video/EncoderFFmpeg.h>

#define STREAM_FRAME_RATE 25 // 25 images/s
#define STREAM_PIX_FMT PIX_FMT_YUV420P // default pix_fmt


typedef struct AVCodecTag {
    int id;
    unsigned int tag;
} AVCodecTag;

namespace vrkit
{

namespace video
{

EncoderFFmpeg::EncoderFFmpeg()
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
{
   /* Do nothing. */ ;
}

EncoderPtr EncoderFFmpeg::create()
{
   return EncoderPtr(new EncoderFFmpeg);
}

#if 0
EncoderFFmpeg::codec_list_t EncoderFFmpeg::getCodecs()
{
   avcodec_register_all();
   av_register_all();

   // Get a list of all codecs.
   codec_list_t codecs;
   for (AVCodec* p = ::first_avcodec; p != NULL; p = p->next)
   {
      // We only care about video encoders.
      if (CODEC_TYPE_VIDEO == p->type && NULL != p->encode)
      {
         codecs.push_back(std::string(p->name));
      }
   }

   return codecs;
}
#endif


EncoderFFmpeg::~EncoderFFmpeg()
{
   stopEncoding();
}

void EncoderFFmpeg::stopEncoding()
{
   // Close each codec.
   if (NULL != mVideoStream)
   {
      closeVideo();
   }

   if (NULL != mAudioStream)
   {
      closeAudio();
   }

   if (NULL != mFormatContext)
   {
      // Write the trailer, if any
      av_write_trailer(mFormatContext);

      // free the streams
      for(unsigned int i = 0; i < mFormatContext->nb_streams; i++)
      {
         av_freep(&mFormatContext->streams[i]->codec);
         av_freep(&mFormatContext->streams[i]);
      }

      if (!(mFormatOut->flags & AVFMT_NOFILE))
      {
         // close the output file
         url_fclose(&mFormatContext->pb);
      }

      // free the stream
      av_free(mFormatContext);
      mFormatContext = NULL;
   }
}

EncoderPtr EncoderFFmpeg::init()
{
   // Get all types registered
   avcodec_register_all();
   av_register_all();

   AVOutputFormat *out_fmt;
   Encoder::codec_list_t allowable_codecs;
   for( out_fmt = ::first_oformat; out_fmt != NULL; out_fmt = out_fmt->next )
   {
      if(out_fmt->name != NULL)
      {
         if( out_fmt->video_codec != CODEC_ID_NONE )
         {
            allowable_codecs.clear();
            if( out_fmt->codec_tag != NULL )
            {
               for ( const AVCodecTag* tag = out_fmt->codec_tag[0];
                     tag->id != CODEC_ID_NONE;
                     ++tag )
               {
                  AVCodec* current_codec = avcodec_find_encoder((CodecID)tag->id);
                  if(current_codec != NULL)
                  {
                     if(current_codec->type == CODEC_TYPE_VIDEO)
                     {
                        allowable_codecs.push_back(std::string(current_codec->name));
                     }
                  }
               }
            }

            ContainerFormatInfo new_format;
            new_format.mFormatName = std::string(out_fmt->name);
            if( out_fmt->long_name != NULL )
            {
               new_format.mFormatLongName = std::string(out_fmt->long_name);
            }
            if( out_fmt->extensions != NULL )
            {
               std::string extensions(out_fmt->extensions);
               std::vector<std::string> temp_vec;
               boost::algorithm::split(temp_vec, extensions,
                                       boost::algorithm::is_any_of(","));

               new_format.mFileExtensions.reserve(temp_vec.size());

               // Ensure that new_format.mFileExtensions contains only
               // non-empty, well-formed file extensions.
               typedef std::vector<std::string>::iterator iter_type;
               for ( iter_type s = temp_vec.begin(); s != temp_vec.end(); ++s )
               {
                  boost::trim(*s);
                  if ( ! (*s).empty() )
                  {
                     new_format.mFileExtensions.push_back(*s);
                  }
               }
            }

            // Keep only unique codec names
            Encoder::codec_list_t::iterator new_end =
               std::unique(allowable_codecs.begin(), allowable_codecs.end());
            allowable_codecs.erase(new_end, allowable_codecs.end());

            new_format.mCodecList = allowable_codecs;
            new_format.mEncoderName = getName();
            mContainerFormatInfoList.push_back(new_format);
         }
      }
   }

   return shared_from_this();
}

void EncoderFFmpeg::startEncoding()
{
#if defined(VRKIT_DEBUG)
   VRKIT_STATUS << "Creating an encoder. file: " << getFilename()
                << " w: " << getWidth() << " h: " << getHeight() << std::endl;
#endif

   try
   {
      // XXX: This should not be hard coded.
      const vpr::Uint32 bitrate = 1400;

      if ( ! getContainerFormat().empty() )
      {
         mFormatOut = guess_format(getContainerFormat().c_str(), NULL, NULL);
      }

      if ( NULL == mFormatOut )
      {
         VRKIT_STATUS << "Container format invalid or not specified.\n"
                      << "Trying to guess container format from file name: "
                      << getFilename() << std::endl;
         mFormatOut = guess_format(NULL, getFilename().c_str(), NULL);
      }

      // If we can't guess the format from the filename, fallback on mpeg.
      if ( NULL == mFormatOut )
      {
         VRKIT_STATUS << "Could not deduce output format from file extension; "
                      << "using AVI." << std::endl;
         mFormatOut = guess_format("avi", NULL, NULL);
      }

      if ( NULL == mFormatOut )
      {
         throw RecordingConfigError("Couldn't find suitable output format.",
                                    VRKIT_LOCATION);
      }

      VRKIT_STATUS << "Using format: " << mFormatOut->name << " ("
                   << mFormatOut->long_name << ")" << std::endl;

      // Allocate the output media context.
      mFormatContext = av_alloc_format_context();
      if ( NULL == mFormatContext )
      {
         throw Exception("Memory error.", VRKIT_LOCATION);
      }

      mFormatContext->oformat   = mFormatOut;
      mFormatContext->max_delay = static_cast<int>(0.7 * AV_TIME_BASE);
      std::strncpy(mFormatContext->filename, getFilename().c_str(),
                   sizeof(mFormatContext->filename));

      // Add the audio and video streams using the default format codecs
      // and initialize the codecs.
      if ( mFormatOut->video_codec == CODEC_ID_NONE )
      {
         throw RecordingConfigError(
            "File format does not support video codec.", VRKIT_LOCATION
         );
      }

      AVCodec* codec = avcodec_find_encoder_by_name(getCodec().c_str());

      // Fall back to using the containers default format
      if ( NULL == codec )
      {
#if defined(VRKIT_DEBUG)
         VRKIT_STATUS << "Invalid codec specified for this container. "
                   << "Falling back to default." << std::endl;
#endif
         codec = avcodec_find_encoder(mFormatOut->video_codec);
      }

      if ( NULL == codec )
      {
         throw RecordingConfigError("Couldn't find video encoder.",
                                    VRKIT_LOCATION);
      }

      addVideoStream(codec);

      if ( mFormatOut->audio_codec != CODEC_ID_NONE )
      {
         addAudioStream();
      }

      // Set the output parameters (must be done even if no
      // parameters).
      if ( av_set_parameters(mFormatContext, NULL) < 0 )
      {
         throw RecordingConfigError("Invalid output format parameters.",
                                    VRKIT_LOCATION);
      }

      dump_format(mFormatContext, 0, getFilename().c_str(), 1);

      // Now that all the parameters are set, we can open the audio and
      // video codecs and allocate the necessary encode buffers.
      if ( NULL != mVideoStream )
      {
         openVideo(codec);
      }

      if ( NULL != mAudioStream )
      {
         openAudio();
      }

      // Open the output file, if needed.
      if (!(mFormatOut->flags & AVFMT_NOFILE))
      {
         if (url_fopen(&mFormatContext->pb, getFilename().c_str(), URL_WRONLY) < 0)
         {
            std::ostringstream msg_stream;
            msg_stream << "Coult not open " << getFilename();
            throw RecordingConfigError(msg_stream.str(), VRKIT_LOCATION);
         }
      }

      // Set the correct bitrate.
      // XXX: What is the purpose of this? A different bit rate value is set
      // in addVideoStream(), and this does not seem to relate to it in any
      // way. Moreover, changing this value does nothing to the quality of the
      // encoded movie, but changing the value set in addVideoStream() does.
      mVideoStream->codec->bit_rate = bitrate * 1000;

      // Write the stream header, if any.
      av_write_header(mFormatContext);
   }
   catch (std::exception& ex)
   {
      std::cerr << "Failed to initialize encoder: " << ex.what() << std::endl;
      throw;
   }
}

// Add an audio output stream
void EncoderFFmpeg::addAudioStream()
{
   // TODO: Add audio support.
/*
   mAudioStream = av_new_stream(mFormatContext, 1);
   if (NULL == mAudioStream)
   {
      throw RecordingException("Could not allocate audio stream.",
                               VRKIT_LOCATION);
   }

   AVCodecContext* acc = mAudioStream->codec;
   acc->codec_id = mFormatOut->audio_codec;
   acc->codec_type = CODEC_TYPE_AUDIO;

   // Put sample parameters
   acc->bit_rate = 64000;
   acc->sample_rate = 44100;
   acc->channels = 2;
*/
}

// Add a video output stream
void EncoderFFmpeg::addVideoStream(AVCodec* codec)
{
   // Create a new video stream to write data to.
   mVideoStream = av_new_stream(mFormatContext, 0);
   if ( NULL == mVideoStream )
   {
      throw RecordingException("Could not allocate video stream.",
                               VRKIT_LOCATION);
   }

   // XXX: Should we be discarding this?
   mVideoStream->discard = AVDISCARD_NONKEY;

   mVideoStream->codec->codec_id   = codec->id;
   mVideoStream->codec->codec_type = CODEC_TYPE_VIDEO;
   avcodec_get_context_defaults2(mVideoStream->codec, CODEC_TYPE_VIDEO);
   // put sample parameters
   // XXX: This should not be hard coded.
   mVideoStream->codec->bit_rate = 8388608;     // 8192 kb/s

   // Resolution must be a multiple of two
   mVideoStream->codec->width  = getWidth();
   mVideoStream->codec->height = getHeight();

   // Time base: this is the fundamental unit of time (in seconds) in terms
   // of which frame timestamps are represented. for fixed-fps content,
   // timebase should be 1/framerate and timestamp increments should be
   // identically 1.
   mVideoStream->codec->time_base.den = getFramesPerSecond();
   mVideoStream->codec->time_base.num = 1;
   // Emit one intra frame every twelve frames at most.
   mVideoStream->codec->gop_size = 12;
   mVideoStream->codec->pix_fmt = STREAM_PIX_FMT;
//   mVideoStream->codec->rate_emu

   if ( mVideoStream->codec->codec_id == CODEC_ID_MPEG2VIDEO )
   {
      // just for testing, we also add B frames
      mVideoStream->codec->max_b_frames = 2;
   }

   if ( mVideoStream->codec->codec_id == CODEC_ID_MPEG1VIDEO )
   {
      // Needed to avoid using macroblocks in which some coeffs overflow.
      // This does not happen with normal video, it just happens here as
      // the motion of the chroma plane does not match the luma plane. */
      mVideoStream->codec->mb_decision = 2;
   }

   // Some formats want stream headers to be separate
   if ( mFormatContext->oformat->flags & AVFMT_GLOBALHEADER )
   {
      mVideoStream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
   }
}

void EncoderFFmpeg::openVideo(AVCodec* codec)
{
   AVCodecContext* vcc = mVideoStream->codec;

   // open the codec
   if ( avcodec_open(vcc, codec) < 0 )
   {
      throw RecordingException("Couldn't open video encoder.",
                               VRKIT_LOCATION);
   }

   mVideoOutBuffer = NULL;
   if ( ! (mFormatContext->oformat->flags & AVFMT_RAWPICTURE) )
   {
      // allocate output buffer
      // XXX: API change will be done
      // buffers passed into lav* can be allocated any way you prefer,
      // as long as they're aligned enough for the architecture, and
      // they're freed appropriately (such as using av_free for buffers
      // allocated with av_malloc)
      mVideoOutBufferSize = getWidth() * getHeight();
      mVideoOutBuffer =
         reinterpret_cast<unsigned char*>(av_malloc(mVideoOutBufferSize));
   }

   // allformat_ctxate the encoded raw picture */
   mYuvFrame = allocFrame(vcc->pix_fmt, vcc->width, vcc->height);
   if ( NULL == mYuvFrame )
   {
      throw RecordingException("Couldn't allocate yuv-picture.",
                               VRKIT_LOCATION);
   }

   mRgbFrame = avcodec_alloc_frame();
   avcodec_get_frame_defaults(mRgbFrame);
   if ( NULL == mRgbFrame )
   {
      throw RecordingException("Couldn't allocate rgb-picture.",
                               VRKIT_LOCATION);
   }
}

void EncoderFFmpeg::closeVideo()
{
   avcodec_close(mVideoStream->codec);

   av_free(mYuvFrame);
   mYuvFrame = NULL;

   av_free(mRgbFrame);
   mRgbFrame = NULL;

   av_free(mVideoOutBuffer);
   mVideoOutBuffer = NULL;

   // XXX: Memory leak.
   mVideoStream = NULL;
}

void EncoderFFmpeg::openAudio()
{
   // TODO: Add audio support.
#if 0
   AVCodecContext* acc = mAudioStream->codec;

   // Find the audio encoder.
   AVCodec* codec = avcodec_find_encoder(acc->codec_id);
   if (!codec)
   {
      throw RecordingException("Audio codec not found.", VRKIT_LOCATION);
   }

   // Open the audio encoder.
   if (avcodec_open(acc, codec) < 0)
   {
      throw RecordingException("Could not open audio codec.", VRKIT_LOCATION);
   }

   // Init signal generator.
   /*
   mT = 0;
   mTincr = 2 * M_PI * 110.0 / acc->sample_rate;
   // Increment frequency by 110 Hz per second.
   mTincr2 = 2 * M_PI * 110.0 / acc->sample_rate / acc->sample_rate;
   */

   mAudioOutBufferSize = 10000;
   mAudioOutBuffer =
      reinterpret_cast<unsigned char*>(av_malloc(mAudioOutBufferSize));

   // Ugly hack for PCM codecs (will be removed ASAP with new PCM
   // support to compute the input frame size in samples.
   if (acc->frame_size <= 1)
   {
      audio_input_frame_size = mAudioOutBufferSize / acc->channels;
      switch(mAudioStream->codec->codec_id)
      {
         case CODEC_ID_PCM_S16LE:
         case CODEC_ID_PCM_S16BE:
         case CODEC_ID_PCM_U16LE:
         case CODEC_ID_PCM_U16BE:
            audio_input_frame_size >>= 1;
            break;
         default:
            break;
      }
   }
   else
   {
      audio_input_frame_size = acc->frame_size;
   }

   mAudioSamples =
      reinterpret_cast<int16_t*>(
         av_malloc(audio_input_frame_size * 2 * acc->channels)
      );
#endif
}

void EncoderFFmpeg::closeAudio()
{
   // TODO: Add audio support.
/*
   avcodec_close(mAudioStream->codec);
   av_free(mAudioSamples);
   av_free(mAudioOutBuffer);

   mAudioSamples = NULL;
   mAudioOutBuffer = NULL;
   mAudioStream = NULL;
*/
}

AVFrame* EncoderFFmpeg::allocFrame(const int pixFormat, const int width,
                                   const int height)
{
   AVFrame* picture = avcodec_alloc_frame();
   avcodec_get_frame_defaults(picture);
   if (NULL == picture)
   {
      throw RecordingException("Error allocating frame.", VRKIT_LOCATION);
   }

   int size = avpicture_get_size(pixFormat, width, height);
   unsigned char* picture_buf = (unsigned char*)av_malloc(size);

   if (NULL == picture_buf)
   {
      av_free(picture);
      throw RecordingException("Error allocating picture buffer.",
                               VRKIT_LOCATION);
   }

   avpicture_fill((AVPicture*)picture, picture_buf, pixFormat, width, height);
   return picture;
}

void EncoderFFmpeg::writeFrame(const vpr::Uint8* data)
{
   double audio_pts, video_pts = 0.0;

   // Compute current audio and video time
   if ( mAudioStream )
   {
      audio_pts = static_cast<double>(mAudioStream->pts.val) *
                     mAudioStream->time_base.num / mAudioStream->time_base.den;
   }
   else
   {
      audio_pts = 0.0;
   }

   if (mVideoStream)
   {
      video_pts = static_cast<double>(mVideoStream->pts.val) *
                     mVideoStream->time_base.num / mVideoStream->time_base.den;
   }
   else
   {
      video_pts = 0.0;
   }

   // Early out if we don't have a video stream.
   if (NULL == mVideoStream)
   {
      return;
   }

   avpicture_fill(reinterpret_cast<AVPicture*>(mRgbFrame),
                  const_cast<vpr::Uint8*>(data), PIX_FMT_RGB24, getWidth(),
                  getHeight());

   // convert rgb to yuv420
   img_convert(reinterpret_cast<AVPicture*>(mYuvFrame),
               mVideoStream->codec->pix_fmt,
               reinterpret_cast<AVPicture*>(mRgbFrame), PIX_FMT_RGB24,
               mVideoStream->codec->width, mVideoStream->codec->height);

   // Flip yuv-buffer horizontal -> opengl has other order
   // why not the rgb-buffer? yuv has smaller size
   //
   // TODO: check pixFormat and do the right thing, its not
   // alway yuf420 ...
   if ( mFlipBeforeEncode )
   {
      unsigned char* s;
      unsigned char* d;

      static unsigned char b[24000];

      for (s= mYuvFrame->data[0], d= mYuvFrame->data[1]-mYuvFrame->linesize[0];
           s < d; s+= mYuvFrame->linesize[0], d-= mYuvFrame->linesize[0])
      {
         memcpy(b, s, mYuvFrame->linesize[0]);
         memcpy(s, d, mYuvFrame->linesize[0]);
         memcpy(d, b, mYuvFrame->linesize[0]);
      }

      for (s= mYuvFrame->data[1], d= mYuvFrame->data[2]-mYuvFrame->linesize[2];
           s < d; s+= mYuvFrame->linesize[1], d-= mYuvFrame->linesize[1])
      {
         memcpy(b, s, mYuvFrame->linesize[1]);
         memcpy(s, d, mYuvFrame->linesize[1]);
         memcpy(d, b, mYuvFrame->linesize[1]);
      }

      for (s= mYuvFrame->data[2], d= mYuvFrame->data[2]+(mYuvFrame->data[2]-mYuvFrame->data[1]-mYuvFrame->linesize[2]);
           s < d; s+= mYuvFrame->linesize[2], d-= mYuvFrame->linesize[2])
      {
         memcpy(b, s, mYuvFrame->linesize[2]);
         memcpy(s, d, mYuvFrame->linesize[2]);
         memcpy(d, b, mYuvFrame->linesize[2]);
      }
   }

   int status = 0;

   if (mFormatContext->oformat->flags & AVFMT_RAWPICTURE)
   {
      // Raw video case. The API will change slightly in the near
      // future for that.
      AVPacket pkt;
      av_init_packet(&pkt);

      pkt.flags |= PKT_FLAG_KEY;
      pkt.stream_index = mVideoStream->index;
      pkt.data = (uint8_t *)mYuvFrame;
      pkt.size = sizeof(AVPicture);

      status = av_interleaved_write_frame(mFormatContext, &pkt);
   }
   else
   {
      AVCodecContext* vcc = mVideoStream->codec;
      // Encode the image
      int out_size = avcodec_encode_video(vcc, mVideoOutBuffer,
                                          mVideoOutBufferSize, mYuvFrame);

      // If zero size, it means the image was buffered.
      if ( out_size > 0 )
      {
         AVPacket pkt;
         av_init_packet(&pkt);

         pkt.pts = av_rescale_q(vcc->coded_frame->pts, vcc->time_base,
                                mVideoStream->time_base);

         if ( vcc->coded_frame->key_frame )
         {
            pkt.flags |= PKT_FLAG_KEY;
         }

         pkt.stream_index = mVideoStream->index;
         pkt.data = mVideoOutBuffer;
         pkt.size = out_size;

         // Write the compressed frame in the media file.
         status = av_interleaved_write_frame(mFormatContext, &pkt);
      }
   }

   if ( status != 0 )
   {
      throw RecordingException("Error while writing frame.", VRKIT_LOCATION);
   }

   //VRKIT_STATUS << "frame: " << mFrameCount << std::endl;
   ++mFrameCount;
}

}

}

#endif /* VRKIT_WITH_FFMPEG */
