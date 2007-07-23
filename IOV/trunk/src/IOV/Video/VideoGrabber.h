// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_VIDEO_GRABBER_H_
#define _INF_VIDEO_GRABBER_H_

#include <IOV/Config.h>

#include <boost/enable_shared_from_this.hpp>

#include <OpenSG/OSGViewport.h>
#include <OpenSG/OSGImage.h>

#include <IOV/Video/VideoGrabberPtr.h>
#include <IOV/Video/FfmpegEncoderPtr.h>

namespace inf
{

/**
 * Copies OpenGL framebuffer from a given viewport into a movie.
 *
 * @since 0.37
 */
class IOV_CLASS_API VideoGrabber : public boost::enable_shared_from_this<VideoGrabber>
{
protected:
   VideoGrabber()
      : mRecording(false)
      , mUseFbo(false)
      , mImage(OSG::NullFC)
      , mViewport(OSG::NullFC)
#ifdef IOV_WITH_FFMPEG
      , mEncoder()
#endif
   {;}

public:
   static VideoGrabberPtr create();

   virtual ~VideoGrabber();

   /**
    * Initialize the video grabber.
    *
    * @param viewport Viewport to grab image from.
    * @param filename Movie file to save data to.
    */
   VideoGrabberPtr init(OSG::ViewportPtr viewport);

   /**
    * Called be viewer each frame to render scene into FBO.
    */
   void draw();

   /**
    * Start recording movie to the given file.
    */
   void record(const std::string& filename);

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

private:
   bool                 mRecording;
   bool                 mUseFbo;
   OSG::ImagePtr        mImage;
   OSG::ViewportPtr     mViewport;
#ifdef IOV_WITH_FFMPEG
   FfmpegEncoderPtr     mEncoder;
#endif
};

}

#endif /*_INF_VIDEO_GRABBER_H_*/
