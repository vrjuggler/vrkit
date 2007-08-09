// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_VIDEO_GRABBER_H_
#define _INF_VIDEO_GRABBER_H_

#include <IOV/Config.h>

#include <boost/function.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <OpenSG/OSGViewport.h>
#include <OpenSG/OSGImage.h>

#include <IOV/Video/VideoEncoderPtr.h>

#include <IOV/Video/EncoderPtr.h>

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
    * @param viewport Viewport to grab image from.
    * @param filename Movie file to save data to.
    */
   VideoEncoderPtr init(OSG::ViewportPtr viewport);

   /**
    * Called be viewer each frame to render scene into FBO.
    */
   void grabFrame(const bool leftEye = true);

   void writeFrame();

   /**
    * Start recording movie to the given file.
    */
   void record(const std::string& filename, const std::string& codec,
               const OSG::UInt32 framesPerSecond = 60, const bool stereo = false);

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
    * Returns the current set of available codes.
    *
    * @since 0.42.1
    */
   const std::set<std::string>& getAvailableCodecs() const
   {
      return mCodecSet;
   }

private:
   typedef std::vector<std::string> encoder_list_t;
   typedef std::map<std::string, encoder_list_t> codec_map_t;
   typedef std::set<std::string> codec_set_t;
   typedef boost::function<EncoderPtr ()> encoder_create_t;
   typedef std::map<std::string, encoder_create_t> creator_map_t;

   bool                 mRecording;     /**< Whether we are currently recording. */
   bool                 mStereo;
   bool                 mUseFbo;        /**< If we are using a FBO or the default pixel buffer. */
   OSG::ImagePtr        mImage;         /**< Image to hold the pixel data while encoding. */
   OSG::ViewportPtr     mViewport;      /**< Viewport that contains source frame buffer. */
   EncoderPtr           mEncoder;       /**< Encoder that can write encode and write movie. */
   codec_map_t          mCodecMap;      /**< Map of codec_name to a list of encoders. */
   creator_map_t        mCreatorMap;    /**< Map of encoder names to encoder creators. */
   codec_set_t		mCodecSet;	/**< Set of all codecs available. */
};

}

#endif /*_INF_VIDEO_GRABBER_H_*/
