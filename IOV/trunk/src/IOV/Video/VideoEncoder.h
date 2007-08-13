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
    * Returns the current set of available codes.
    *
    * @since 0.42.1
    */
   const std::set<std::string>& getAvailableCodecs() const
   {
      return mCodecSet;
   }

   void setFilename(const std::string& filename);

   void setCodec(const std::string& codec);

   void setStereo(bool isStereo);

   void setSize(OSG::UInt32 width, OSG::UInt32 height);

   void setFramesPerSecond(OSG::UInt32 fps);

private:
   typedef std::vector<std::string> encoder_list_t;
   typedef std::map<std::string, encoder_list_t> codec_map_t;
   typedef std::set<std::string> codec_set_t;
   typedef boost::function<EncoderPtr ()> encoder_create_t;
   typedef std::map<std::string, encoder_create_t> creator_map_t;

   bool                 mRecording;     /**< Whether we are currently recording. */
   OSG::ImagePtr        mImage;		/**< Image to hold the pixel data while encoding. */
   bool                 mStereo;
   std::string		mFilename;
   std::string		mCodec;
   OSG::UInt32		mFps;
   OSG::UInt32		mWidth;
   OSG::UInt32		mHeight;
   EncoderPtr           mEncoder;       /**< Encoder that can write encode and write movie. */
   codec_map_t          mCodecMap;      /**< Map of codec_name to a list of encoders. */
   creator_map_t        mCreatorMap;    /**< Map of encoder names to encoder creators. */
   codec_set_t		mCodecSet;	/**< Set of all codecs available. */
};

}

#endif /*_INF_VIDEO_GRABBER_H_*/
