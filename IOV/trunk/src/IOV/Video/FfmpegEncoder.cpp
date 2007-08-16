// Copyright (C) Infiscape Corporation 2005-2007

#ifdef IOV_WITH_FFMPEG

#include <string>
#include <exception>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <IOV/Video/FfmpegEncoder.h>
#include <IOV/Util/Exceptions.h>

#define STREAM_FRAME_RATE 25 // 25 images/s
#define STREAM_PIX_FMT PIX_FMT_YUV420P // default pix_fmt

namespace inf
{

VideoFfmpegEncoderException::VideoFfmpegEncoderException(const std::string& error) throw()
   : s("FfmpegEncoder error: ")
{ s += error; }

VideoFfmpegEncoderException::~VideoFfmpegEncoderException() throw()
{;}

const char* VideoFfmpegEncoderException::what(void) const throw()
{ return s.c_str(); }


EncoderPtr FfmpegEncoder::create()
{
   return EncoderPtr(new FfmpegEncoder);
}

FfmpegEncoder::codec_list_t FfmpegEncoder::getCodecs()
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


FfmpegEncoder::~FfmpegEncoder()
{
   close();
}

void FfmpegEncoder::close()
{
   // Close each codec.
   if (NULL != mVideoStream)
   {
      closeVideo();
   }
   // XXX:Don't add audio for now,
   /*
   if (NULL != mAudioStream)
   {
      closeAudio();
   }
   */

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

static void show_formats(void)
{
    AVInputFormat *ifmt;
    AVOutputFormat *ofmt;
    AVCodec *p, *p2;
    const char *last_name;

    printf("File formats:\n");
    last_name= "000";
    for(;;){
        int decode=0;
        int encode=0;
        const char *name=NULL;
        const char *long_name=NULL;
	const char *exts=NULL;

        for(ofmt = ::first_oformat; ofmt != NULL; ofmt = ofmt->next) {
            if((name == NULL || strcmp(ofmt->name, name)<0) &&
                strcmp(ofmt->name, last_name)>0){
                name= ofmt->name;
                long_name= ofmt->long_name;
		exts=ofmt->extensions;
                encode=1;
            }
        }
        for(ifmt = first_iformat; ifmt != NULL; ifmt = ifmt->next) {
            if((name == NULL || strcmp(ifmt->name, name)<0) &&
                strcmp(ifmt->name, last_name)>0){
                name= ifmt->name;
                long_name= ifmt->long_name;
                encode=0;
            }
            if(name && strcmp(ifmt->name, name)==0)
                decode=1;
        }
        if(name==NULL)
            break;
        last_name= name;
        if(encode)
	{
        printf(
            "%-20s  %-15s %s\n",
            exts,
	    name,
            long_name ? long_name:" ");
	 }
    }
    printf("\n");

    printf("Codecs:\n");
    last_name= "000";
    for(;;){
        int decode=0;
        int encode=0;
        int cap=0;
        const char *type_str;

        p2=NULL;
        for(p = first_avcodec; p != NULL; p = p->next)
        {
            if((p2==NULL || strcmp(p->name, p2->name)<0) && strcmp(p->name, last_name)>0)
            {
                p2= p;
                decode= encode= cap=0;
            }
            if(p2 && strcmp(p->name, p2->name)==0)
            {
                if(p->decode)
                   decode=1;
                if(p->encode)
                   encode=1;
                cap |= p->capabilities;
            }
        }
        if(p2==NULL)
            break;
        last_name= p2->name;

        switch(p2->type) {
        case CODEC_TYPE_VIDEO:
            type_str = "V";
            break;
        case CODEC_TYPE_AUDIO:
            type_str = "A";
            break;
        case CODEC_TYPE_SUBTITLE:
            type_str = "S";
            break;
        default:
            type_str = "?";
            break;
        }
	if(encode)
	{
        printf(
            " %s%s%s%s%s%s %s",
            decode ? "D": (/*p2->decoder ? "d":*/" "),
            encode ? "E":" ",
            type_str,
            cap & CODEC_CAP_DRAW_HORIZ_BAND ? "S":" ",
            cap & CODEC_CAP_DR1 ? "D":" ",
            cap & CODEC_CAP_TRUNCATED ? "T":" ",
            p2->name);
       /* if(p2->decoder && decode==0)
            printf(" use %s for decoding", p2->decoder->name);*/
        printf("\n");
	}
    }
    printf("\n");
}

EncoderPtr FfmpegEncoder::init(const std::string& filename, const std::string& codecName,
                               const vpr::Uint32 width, const vpr::Uint32 height,
                               const vpr::Uint32 framesPerSecond)
{
   std::cout << "Creating a encoder. file: " << filename
      << " w: " << width << " h: " << height << std::endl;
   try
   {
      const vpr::Uint32 bitrate = 1400;

      // init avcodec && avformat
      avcodec_register_all();
      av_register_all();

      // XXX: Debug code to output all valid formats & codecs.
      show_formats();
      
      std::cout << "Trying to guess container format from filename: " << filename << std::endl;
      mFormatOut = guess_format(NULL, filename.c_str(), NULL);

      // If we can't guess the format from the filename, fallback on mpeg.
      if (NULL == mFormatOut)
      {
         std::cout << "Could not deduce output format from file extension: using MPEG." << std::endl;
         mFormatOut = guess_format("avi", NULL, NULL);
      }
      if (NULL == mFormatOut)
      {
         throw VideoFfmpegEncoderException("Couldn't find suitable output format.");
      }

      std::cout << "Using format: " << mFormatOut->name << ": " << mFormatOut->long_name << std::endl;

      // Allocate the output media context.
      mFormatContext = av_alloc_format_context();
      if (NULL == mFormatContext)
      {
         throw VideoFfmpegEncoderException("Memory error.");
      }
      mFormatContext->oformat = mFormatOut;
      mFormatContext->max_delay = (int)(0.7 * AV_TIME_BASE);
      snprintf(mFormatContext->filename, sizeof(mFormatContext->filename), "%s", filename.c_str());

      // Add the audio and video streams using the default format codecs
      // and initialize the codecs.
      if (mFormatOut->video_codec == CODEC_ID_NONE)
      {
         throw inf::Exception("File format does not support video codec.", IOV_LOCATION);
      }

      AVCodec* codec = avcodec_find_encoder_by_name(codecName.c_str());

      // Fall back to using the containers default format
      if( NULL == codec )
      {
	 codec = avcodec_find_encoder(mFormatOut->video_codec);
      }

      if (NULL == codec)
      {
         throw VideoFfmpegEncoderException("Couldn't find video encoder.");
      }

      addVideoStream(width, height, framesPerSecond, codec);

      // XXX:Don't add audio for now,
      /*
      if (mFormatOut->audio_codec != CODEC_ID_NONE)
      {
         addAudioStream();
      }
      */

      // Set the output parameters (must be done even if no
      // parameters).
      if (av_set_parameters(mFormatContext, NULL) < 0)
      {
         throw VideoFfmpegEncoderException("Invalid output format parameters.");
      }

      dump_format(mFormatContext, 0, filename.c_str(), 1);

      // Now that all the parameters are set, we can open the audio and
      // video codecs and allocate the necessary encode buffers.
      if (NULL != mVideoStream)
      {
         openVideo(codec);
      }

      // XXX:Don't add audio for now,
      /*
      if (NULL != mAudioStream)
      {
         openAudio();
      }
      */

      // Open the output file, if needed.
      if (!(mFormatOut->flags & AVFMT_NOFILE))
      {
         if (url_fopen(&mFormatContext->pb, filename.c_str(), URL_WRONLY) < 0)
         {
            std::string err = "Could not open" + filename;
            throw VideoFfmpegEncoderException(err);
         }
      }

      // Set the correct bitrate.
      mVideoStream->codec->bit_rate = bitrate * 1000;

      // Write the stream header, if any.
      av_write_header(mFormatContext);
   }
   catch(std::exception& ex)
   {
      std::cout << "Failed to initialize encoder: " << ex.what() << std::endl;
      throw;
   }

   return shared_from_this();
}

// Add an audio output stream
void FfmpegEncoder::addAudioStream()
{
   mAudioStream = av_new_stream(mFormatContext, 1);
   if (NULL == mAudioStream)
   {
      throw VideoFfmpegEncoderException("Could not allocate audio stream.");
   }

   AVCodecContext* acc = mAudioStream->codec;
   acc->codec_id = mFormatOut->audio_codec;
   acc->codec_type = CODEC_TYPE_AUDIO;

   // Put sample parameters
   acc->bit_rate = 64000;
   acc->sample_rate = 44100;
   acc->channels = 2;
}

// Add a video output stream
void FfmpegEncoder::addVideoStream(const vpr::Uint32 width, const vpr::Uint32 height,
                                   const vpr::Uint32 fps, AVCodec* codec)
{
   // Create a new video stream to write data to.
   mVideoStream = av_new_stream(mFormatContext, 0);
   if (NULL == mVideoStream)
   {
      throw VideoFfmpegEncoderException("Could not allocate video stream.");
   }

   // XXX: Should we be discarding this?
   mVideoStream->discard = AVDISCARD_NONKEY;

   AVCodecContext* vcc = mVideoStream->codec;
   vcc->codec_id = codec->id;
   vcc->codec_type = CODEC_TYPE_VIDEO;
   avcodec_get_context_defaults2(mVideoStream->codec, CODEC_TYPE_VIDEO);
   // put sample parameters
   vcc->bit_rate = 400000;

   // Resolution must be a multiple of two
   vcc->width = width;
   vcc->height = height;

   // Time base: this is the fundamental unit of time (in seconds) in terms
   // of which frame timestamps are represented. for fixed-fps content,
   // timebase should be 1/framerate and timestamp increments should be
   // identically 1.
   //vcc->time_base.den = STREAM_FRAME_RATE;
   vcc->time_base.den = fps;
   vcc->time_base.num = 1;
   //vcc->discard = AVDISCARD_NONKEY;
   vcc->gop_size = 12; /* emit one intra frame every twelve frames at most */
   vcc->pix_fmt = STREAM_PIX_FMT;
//   vcc->rate_emu
   if (vcc->codec_id == CODEC_ID_MPEG2VIDEO)
   {
      // just for testing, we also add B frames
      vcc->max_b_frames = 2;
   }
   if (vcc->codec_id == CODEC_ID_MPEG1VIDEO)
   {
      // Needed to avoid using macroblocks in which some coeffs overflow.
      // This does not happen with normal video, it just happens here as
      // the motion of the chroma plane does not match the luma plane. */
      vcc->mb_decision=2;
   }

   // Some formats want stream headers to be separate
   if(mFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
   {
      vcc->flags |= CODEC_FLAG_GLOBAL_HEADER;
   }
}

void FfmpegEncoder::openVideo(AVCodec* codec)
{
   AVCodecContext* vcc = mVideoStream->codec;

   // open the codec
   if (avcodec_open(vcc, codec) < 0)
   {
      throw VideoFfmpegEncoderException("Couldn't open video encoder.");
   }

   mVideoOutBuffer = NULL;
   if (!(mFormatContext->oformat->flags & AVFMT_RAWPICTURE))
   {
      // allocate output buffer
      // XXX: API change will be done
      // buffers passed into lav* can be allocated any way you prefer,
      // as long as they're aligned enough for the architecture, and
      // they're freed appropriately (such as using av_free for buffers
      // allocated with av_malloc)
      mVideoOutBufferSize = width() * height();
      mVideoOutBuffer= (unsigned char*)av_malloc(mVideoOutBufferSize);
   }


   // allformat_ctxate the encoded raw picture */
   mYuvFrame= allocFrame(vcc->pix_fmt, vcc->width, vcc->height);
   if (NULL == mYuvFrame)
   {
      throw VideoFfmpegEncoderException("Couldn't allocate yuv-picture.");
   }

   mRgbFrame= avcodec_alloc_frame();
   avcodec_get_frame_defaults(mRgbFrame);
   if (NULL == mRgbFrame)
   {
      throw VideoFfmpegEncoderException("Couldn't allocate rgb-picture.");
   }
}

void FfmpegEncoder::closeVideo()
{
   avcodec_close(mVideoStream->codec);

   av_free(mYuvFrame->data[0]);
   av_free(mYuvFrame);
   mYuvFrame = NULL;

   av_free(mRgbFrame->data[0]);
   av_free(mRgbFrame);
   mRgbFrame = NULL;

   av_free(mVideoOutBuffer);
   mVideoOutBuffer = NULL;

   mVideoStream = NULL;
}

void FfmpegEncoder::openAudio()
{
   AVCodecContext* acc = mAudioStream->codec;

   // Find the audio encoder.
   AVCodec* codec = avcodec_find_encoder(acc->codec_id);
   if (!codec)
   {
      throw VideoFfmpegEncoderException("Audio codec not found.");
   }

   // Open the audio encoder.
   if (avcodec_open(acc, codec) < 0)
   {
      throw VideoFfmpegEncoderException("Could not open audio codec.");
   }

   // Init signal generator.
   /*
   mT = 0;
   mTincr = 2 * M_PI * 110.0 / acc->sample_rate;
   // Increment frequency by 110 Hz per second.
   mTincr2 = 2 * M_PI * 110.0 / acc->sample_rate / acc->sample_rate;
   */

   mAudioOutBufferSize = 10000;
   mAudioOutBuffer = (unsigned char*)av_malloc(mAudioOutBufferSize);

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
   mAudioSamples = (int16_t*)av_malloc(audio_input_frame_size * 2 * acc->channels);
}

void FfmpegEncoder::closeAudio()
{
   avcodec_close(mAudioStream->codec);
   av_free(mAudioSamples);
   av_free(mAudioOutBuffer);

   mAudioSamples = NULL;
   mAudioOutBuffer = NULL;
   mAudioStream = NULL;
}

AVFrame* FfmpegEncoder::allocFrame(int pixFormat, int width, int height)
{
   AVFrame* picture = avcodec_alloc_frame();
   avcodec_get_frame_defaults(picture);
   if (NULL == picture)
   {
      throw VideoFfmpegEncoderException("Error allocating frame.");
   }

   int size = avpicture_get_size(pixFormat, width, height);
   unsigned char* picture_buf = (unsigned char*)av_malloc(size);

   if (NULL == picture_buf)
   {
      av_free(picture);
      throw VideoFfmpegEncoderException("Error allocating picture buffer.");
   }

   avpicture_fill((AVPicture*)picture, picture_buf, pixFormat, width, height);
   return picture;
}

void FfmpegEncoder::writeFrame(int width, int height, vpr::Uint8* data)
{
   double audio_pts, video_pts = 0.0;

   // Compute current audio and video time
   if (mAudioStream)
   {
      audio_pts = (double)mAudioStream->pts.val * mAudioStream->time_base.num / mAudioStream->time_base.den;
   }
   else
   {
      audio_pts = 0.0;
   }

   if (mVideoStream)
   {
      video_pts = (double)mVideoStream->pts.val * mVideoStream->time_base.num / mVideoStream->time_base.den;
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

   avpicture_fill((AVPicture*)mRgbFrame, data, PIX_FMT_RGB24, width, height);

   // convert rgb to yuv420
   img_convert( (AVPicture*)(mYuvFrame), mVideoStream->codec->pix_fmt,
                (AVPicture*)(mRgbFrame), PIX_FMT_RGB24,
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

      static unsigned char  b[24000];

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
      int out_size = avcodec_encode_video(vcc, mVideoOutBuffer, mVideoOutBufferSize, mYuvFrame);
      // If zero size, it means the image was buffered.
      if (out_size > 0)
      {
         AVPacket pkt;
         av_init_packet(&pkt);

         pkt.pts = av_rescale_q(vcc->coded_frame->pts, vcc->time_base, mVideoStream->time_base);
         if(vcc->coded_frame->key_frame)
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

   if (status != 0)
   {
      throw VideoFfmpegEncoderException("Error while writing frame.");
   }

   //std::cout << "frame: " << mFrameCount << std::endl;
   mFrameCount++;
}

}

#endif /*IOV_WITH_FFMPEG*/
