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

#ifndef _VRKIT_VIDEO_ENCODER_MANAGER_H_
#define _VRKIT_VIDEO_ENCODER_MANAGER_H_

#include <vrkit/Config.h>

#include <map>
#include <boost/enable_shared_from_this.hpp>
#include <boost/signal.hpp>

#include <OpenSG/OSGImage.h>

#include <vrkit/signal/Proxy.h>
#include <vrkit/video/Encoder.h>
#include <vrkit/video/EncoderManagerPtr.h>


namespace vrkit
{

namespace video
{

/**
 * Copies OpenGL framebuffer from a given viewport into a movie.
 *
 * @note This class was renamed from vrkit::VideoEncoder and moved into the
 *       vrkit::video namespace in version 0.47.
 *
 * @since 0.37
 */
class VRKIT_CLASS_API EncoderManager
   : public boost::enable_shared_from_this<EncoderManager>
{
protected:
   EncoderManager();

public:
   static EncoderManagerPtr create();

   virtual ~EncoderManager();

   /**
    * Initialize the video grabber.
    *
    */
   EncoderManagerPtr init();

   void writeFrame(OSG::ImagePtr img);

   /**
    * Start recording movie to the given file.
    *
    * @ returns True if recording was successfully started.
    */
   bool record();

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
    * Returns if we are recording but paused. If the returned value is true,
    * then isRecording() will also return true.
    *
    * @since 0.49.3
    */
   bool isPaused() const
   {
      return mPaused;
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

   void setStereo(const bool isStereo);

   /**
    * @since 0.49.3
    */
   bool inStereo() const
   {
      return mStereo;
   }

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
    *      vrkit::Encoder object held by this object.
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
   signal::Proxy<basic_signal_t> encodingStarted()
   {
      return signal::Proxy<basic_signal_t>(mEncodingStarted);
   }

   /**
    * Signal emitted when encoding is paused.
    *
    * @since 0.45.2
    */
   signal::Proxy<basic_signal_t> encodingPaused()
   {
      return signal::Proxy<basic_signal_t>(mEncodingPaused);
   }

   /**
    * Signal emitted when encoding resumes.
    *
    * @since 0.45.2
    */
   signal::Proxy<basic_signal_t> encodingResumed()
   {
      return signal::Proxy<basic_signal_t>(mEncodingResumed);
   }

   /**
    * Signal emitted when encoding stops.
    *
    * @since 0.45.2
    */
   signal::Proxy<basic_signal_t> encodingStopped()
   {
      return signal::Proxy<basic_signal_t>(mEncodingStopped);
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
   bool                 mPaused;        /**< Whether we are currently paused while recording. */
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

}


#endif /* _VRKIT_VIDEO_ENCODER_MANAGER_H_ */
