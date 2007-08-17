// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_VIDEO_GRABBER_H_
#define _INF_VIDEO_GRABBER_H_

#include <IOV/Config.h>

#include <boost/function.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <OpenSG/OSGViewport.h>
#include <OpenSG/OSGImage.h>

#include <IOV/Video/VideoEncoderPtr.h>

#include <IOV/Video/Encoder.h>

#include <set>

namespace inf
{

/**
 * Copies OpenGL framebuffer from a given viewport into a movie.
 *
 * @since 0.37
 */
class IOV_CLASS_API VideoEncoder : public boost::enable_shared_from_this<VideoEncoder>
{
protected:
   VideoEncoder();

public:
   static VideoEncoderPtr create();

   virtual ~VideoEncoder();

   /**
    * Initialize the video grabber.
    *
    */
   VideoEncoderPtr init();

   void writeFrame(OSG::ImagePtr img);

   /**
    * Start recording movie to the given file.
    */
   void record();

   /**
    * Pause the recording.
    */
   void pause();

   /**
    * Resume recording.
    */
   void resume();

   /**
    * Stop recording the scene.
    */
   void stop();

   /**
    * Returns if we are recording.
    */
   bool isRecording()
   {
      return mRecording;
   }

   /**
    * Returns the current set of available container formats
    * with their supported codecs.
    *
    */
   const Encoder::container_format_list_t& getAvailableFormats() const
   {
      return mVideoEncoderFormatList;
   }

   void setFilename(const std::string& filename);

   void setStereo(bool isStereo);

   void setSize(OSG::UInt32 width, OSG::UInt32 height);

   void setFramesPerSecond(OSG::UInt32 fps);

   struct video_encoder_format_t
   {
      std::string	mEncoderName;
      std::string	mContainerFormat;
      std::string	mCodec;
   };

   void setFormat(const video_encoder_format_t& format);

private:
   typedef std::map<std::string, EncoderPtr> encoder_map_t;

   bool                 mRecording;     /**< Whether we are currently recording. */
   OSG::ImagePtr        mImage;		/**< Image to hold the pixel data while encoding. */
   bool                 mStereo;
   std::string		mFilename;
   OSG::UInt32		mFps;
   OSG::UInt32		mWidth;
   OSG::UInt32		mHeight;
   EncoderPtr           mEncoder;       /**< Encoder that can write encode and write movie. */

   encoder_map_t			mEncoderMap;
   video_encoder_format_t		mVideoEncoderParams;
   Encoder::container_format_list_t	mVideoEncoderFormatList;
};

}

#endif /*_INF_VIDEO_GRABBER_H_*/
