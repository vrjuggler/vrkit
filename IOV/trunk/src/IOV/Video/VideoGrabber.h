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
 */
class IOV_CLASS_API VideoGrabber : public boost::enable_shared_from_this<VideoGrabber>
{
protected:
   VideoGrabber()
      : mEnabled(true)
      , mUseFbo(false)
      , mFilename()
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
   VideoGrabberPtr init(OSG::ViewportPtr viewport, const std::string& filename);

   void draw();

private:
   bool                 mEnabled;
   bool                 mUseFbo;
   std::string          mFilename;
   OSG::ImagePtr        mImage;
   OSG::ViewportPtr     mViewport;
#ifdef IOV_WITH_FFMPEG
   FfmpegEncoderPtr     mEncoder;
#endif
};

}

#endif /*_INF_VIDEO_GRABBER_H_*/
