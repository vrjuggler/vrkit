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

#ifndef _VRKIT_VIDEO_RECORDER_H_
#define _VRKIT_VIDEO_RECORDER_H_

#include <vrkit/Config.h>

#include <map>
#include <vector>
#include <boost/enable_shared_from_this.hpp>
#include <boost/signal.hpp>

#include <OpenSG/OSGImage.h>
#include <OpenSG/OSGTransform.h>
#include <OpenSG/OSGWindow.h>
#include <OpenSG/OSGRenderAction.h>

#include <vrkit/signal/Proxy.h>
#include <vrkit/video/CameraPtr.h>
#include <vrkit/video/RecorderPtr.h>
#include <vrkit/video/Encoder.h>


namespace vrkit
{

namespace video
{

/** \class Recorder Recorder.h vrkit/video/Recorder.h
 *
 * Records a scene with a camera and video encoder to a file. This is the
 * primary interface to the vrkit video capture features.
 *
 * @note This class was renamed from vrkit::VideoCamera and moved into the
 *       vrkit::video namespace in version 0.47.
 *
 * @since 0.43
 */
class VRKIT_CLASS_API Recorder
   : public boost::enable_shared_from_this<Recorder>
{
protected:
   Recorder();

public:
   static RecorderPtr create();

   /**
    * Initializes this video recorder.
    *
    * @return A pointer to this object is returned as a
    *         vrkit::video::RecorderPtr object.
    */
   RecorderPtr init();

   ~Recorder();

   /** @name Video Encoder Format Management */
   //@{
   /**
    * Returns the current set of available container formats with their
    * supported codecs.
    */
   const Encoder::container_format_list_t& getAvailableFormats() const
   {
      return mVideoEncoderFormatList;
   }

   /**
    * @since 0.50.0
    */
   struct VideoEncoderFormat
   {
      std::string mEncoderName;         /**< The name of the encoder to use */
      std::string mContainerFormat;     /**< The container format for the output file */
      std::string mCodec;               /**< The codec to use for encoding the rendered frames */
   };

   void setFormat(const VideoEncoderFormat& format);
   //@}

   /** @name Movie Attributes */
   //@{
   /*
    * Set the filename to record the video to.
    */
   void setFilename(const std::string& filename);

   /**
    * Set the field of view for the recorder in degrees.
    */
   void setFov(const OSG::Real32 fov);

   /**
    * Set the aspect ration for the recorder.
    */
   void setAspect(const OSG::Real32 aspect);

   /*
    * Set the frames per second that should be recorded.
    */
   void setFramesPerSecond(const OSG::UInt32 framesPerSecond);

   /**
    * Set the size of a video frame.
    */
   void setFrameSize(const OSG::UInt32 width, const OSG::UInt32 height);

   /**
    * Set the interocular distance used for stereo rendering.
    */
   void setInterocularDistance(const OSG::Real32 interocular);

   /**
    * Set the near and far plane for the recorder.
    */
   void setNearFar(const OSG::Real32 nearVal, const OSG::Real32 farVal);

   /**
    * Set the root of the scene graph to record.
    */
   void setSceneRoot(OSG::NodePtr root);

   /**
    * Set the draw scale factor so we can generate correct visual aids
    */
   void setDrawScaleFactor(const float scale);

   /**
    * Set whether to record video in stereo.
    */
   void setStereo(const bool stereo);

   /**
    * @since 0.50.0
    */
   bool inStereo() const
   {
      return mStereo;
   }

   /**
    * Set the traversal mask for rendering if required.
    */
   void setTravMask(const OSG::UInt32 value);
   //@}

   /** @name Recording Controls */
   //@{
   /**
    * Use the current configured settings to start recording video.
    */
   void startRecording();

   /**
    * Pause the current recording.
    *
    * @pre Recording has started.
    */
   void pause();

   /**
    * Resume recording.
    *
    * @pre Recording has started and was paused.
    */
   void resume();

   /**
    * End the current recording.
    */
   void endRecording();

   /**
    * Returns whether the recorder has started recording and has not ended.
    */
   bool isRecording() const
   {
      return mRecording;
   }

   /**
    * Returns whether the recorder is recording and paused.
    */
   bool isPaused() const
   {
      return mPaused;
   }
   //@}

   /** @name Rendering Interface */
   //@{
   /**
    * Sets the OpenSG window for the internal camera. This must be called
    * during the initialization of an OpenGL context. Likely places for this
    * are an override of vrkit::Viewer::contextInit() or the implementation
    * of vrkit::Plugin::contextInit().
    */
   void contextInit(OSG::WindowPtr window);

   /*
    * Renders the current frame given a RenderAction and camera position.
    * This must be invoked with an active OpenGL context. Likely places for
    * making this call are overrides of the vrkit::Viewer draw methods
    * (contextPreDraw(), draw(), and contextPostDraw()) or vrkit::Plugin draw
    * methods.
    */
   void render(OSG::RenderAction* ra, const OSG::Matrix& camPos);
   //@}

   /** @name Recording Visual Cues */
   //@{
   /**
    * Get a debug node that contains a plane with the Camera's texture
    * applied to it.
    */
   OSG::NodePtr getDebugPlane() const;

   /**
    * Set the border width for the debug frame.
    */
   void setDebugFrameBorderWidth(const float value);

   /**
    * Set the distance from the view the debug frame is.
    */
   void setDebugFrameDistance(const float value);

   /**
    * Returns the root of the frame that surrounds what will be captured
    * in the recorder.
    */
   OSG::NodePtr getFrame() const;
   //@}

   /** @name Signal Accessors */
   //@{
   typedef boost::signal<void ()> basic_signal_t;

   /**
    * Signal emitted when recording starts.
    *
    * @since 0.45.2
    */
   signal::Proxy<basic_signal_t> recordingStarted()
   {
      return signal::Proxy<basic_signal_t>(mRecordingStarted);
   }

   /**
    * Signal emitted when recording is paused.
    *
    * @since 0.45.2
    */
   signal::Proxy<basic_signal_t> recordingPaused()
   {
      return signal::Proxy<basic_signal_t>(mRecordingPaused);
   }

   /**
    * Signal emitted when recording resumes after being paused.
    *
    * @since 0.45.2
    */
   signal::Proxy<basic_signal_t> recordingResumed()
   {
      return signal::Proxy<basic_signal_t>(mRecordingResumed);
   }

   /**
    * Signal emitted when recording stops.
    *
    * @since 0.45.2
    */
   signal::Proxy<basic_signal_t> recordingStopped()
   {
      return signal::Proxy<basic_signal_t>(mRecordingStopped);
   }
   //@}

private:
   /**
    * Chooses the encoder to use and prepares it for encoding.
    *
    * @return True is returned if recording was successfully started.
    */
   bool startEncoder();

   /**
    * Set the position of the camera.
    */
   void setCameraPos(const OSG::Matrix& camPos);

   void writeFrame(OSG::ImagePtr img);

   void generateDebugFrame();

   CameraPtr            mCamera;        /**< Camera used for rendering. */
   OSG::ImagePtr        mStereoImageStorage; /**< Temp storage for stereo image concatenation. */
   OSG::TransformRefPtr mTransform;     /**< The location and orientation of the camera. */
   OSG::NodeRefPtr      mFrameRoot;     /**< The frame that surrounds the captured scene. */
   OSG::Real32          mEyeOffset;     /**< Interocular distance / 2 for stereo. */
   OSG::Real32          mBorderSize;    /**< The width of the frame geometry. */
   OSG::Real32          mFrameDist;     /**< The distance between the camera and the frame. */
   float                mDrawScale;     /**< Draw scale factor for the scene. */

   /** @name Signal Objects */
   //@{
   basic_signal_t mRecordingStarted;
   basic_signal_t mRecordingPaused;
   basic_signal_t mRecordingResumed;
   basic_signal_t mRecordingStopped;
   //@}

   bool         mRecording;     /**< Whether we are currently recording. */
   bool         mPaused;        /**< Whether we are currently paused while recording. */
   bool         mStereo;
   std::string  mFilename;
   OSG::UInt32  mFps;
   OSG::UInt32  mWidth;
   OSG::UInt32  mHeight;
   EncoderPtr   mEncoder;       /**< The current video encoder. */

   typedef std::map<std::string, EncoderPtr> encoder_map_t;

   encoder_map_t                        mEncoderMap;
   VideoEncoderFormat                   mVideoEncoderParams;
   Encoder::container_format_list_t     mVideoEncoderFormatList;
};

}

}


#endif /* _VRKIT_VIDEO_RECORDER_H_ */
