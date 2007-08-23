// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_VIDEO_GRABBER_H_
#define _INF_VIDEO_GRABBER_H_

#include <IOV/Config.h>

#include <map>
#include <boost/enable_shared_from_this.hpp>
#include <boost/signal.hpp>

#include <OpenSG/OSGImage.h>

#include <IOV/Util/SignalProxy.h>
#include <IOV/Video/VideoEncoderPtr.h>
#include <IOV/Video/Encoder.h>


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
   bool isRecording() const
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

   void setSize(const OSG::UInt32 width, const OSG::UInt32 height);

   void setFramesPerSecond(OSG::UInt32 fps);

   struct video_encoder_format_t
   {
      std::string	mEncoderName;
      std::string	mContainerFormat;
      std::string	mCodec;
   };

   void setFormat(const video_encoder_format_t& format);

   /**
    * Returns the pixel format that is to be used by this encoder.
    *
    * @pre record() has been called so that there is an configured
    *      inf::Encoder object held by this object.
    */
   OSG::Image::PixelFormat getPixelFormat() const;

   /** @name Signal Accessors */
   //@{
   typedef boost::signal<void ()> basic_signal_t;

   /**
    * Signal emitted when encoding starts.
    *
    * @since 0.45.2
    */
   SignalProxy<basic_signal_t> encodingStarted()
   {
      return SignalProxy<basic_signal_t>(mEncodingStarted);
   }

   /**
    * Signal emitted when encoding is paused.
    *
    * @since 0.45.2
    */
   SignalProxy<basic_signal_t> encodingPaused()
   {
      return SignalProxy<basic_signal_t>(mEncodingPaused);
   }

   /**
    * Signal emitted when encoding resumes.
    *
    * @since 0.45.2
    */
   SignalProxy<basic_signal_t> encodingResumed()
   {
      return SignalProxy<basic_signal_t>(mEncodingResumed);
   }

   /**
    * Signal emitted when encoding stops.
    *
    * @since 0.45.2
    */
   SignalProxy<basic_signal_t> encodingStopped()
   {
      return SignalProxy<basic_signal_t>(mEncodingStopped);
   }
   //@}

private:
   /** @name Signal Objects */
   //@{
   basic_signal_t mEncodingStarted;
   basic_signal_t mEncodingPaused;
   basic_signal_t mEncodingResumed;
   basic_signal_t mEncodingStopped;
   //@}

   typedef std::map<std::string, EncoderPtr> encoder_map_t;

   bool                 mRecording;     /**< Whether we are currently recording. */
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
