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

#include <vector>
#include <boost/enable_shared_from_this.hpp>

#include <OpenSG/OSGTransform.h>
#include <OpenSG/OSGWindow.h>
#include <OpenSG/OSGRenderAction.h>

#include <vrkit/video/CameraPtr.h>
#include <vrkit/video/EncoderManager.h>
#include <vrkit/video/RecorderPtr.h>


namespace vrkit
{

namespace video
{

/**
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

   virtual ~Recorder();

   /**
    * Initialize the recorder.
    */
   RecorderPtr init();

   /**
    * Called from the Viewer's context init so that we can set the
    * correct window.
    */
   void contextInit(OSG::WindowPtr window);

   /**
    * Returns the current set of available codes.
    */
   const Encoder::container_format_list_t& getAvailableFormats() const;

   /**
    * Set the codec that the encoder should use.
    */
   void setFormat(const EncoderManager::video_encoder_format_t& format);

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
    * Set whether to record video in stereo.
    */
   void setStereo(const bool stereo);

   /**
    * Set the traversal mask for rendering if required.
    */
   void setTravMask(const OSG::UInt32 value);

   /**
    * Use the current configured settings to start recording video.
    */
   void startRecording();

   /**
    * End the current recording.
    */
   void endRecording();

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
    * Returns whether the recorder has started recording and has not ended.
    */
   bool isRecording() const;

   /**
    * Returns whether the recorder is recording and paused.
    */
   bool isPaused() const;

   /*
    * Renders the current frame given a RenderAction and camera position.
    */
   void render(OSG::RenderAction* ra, const OSG::Matrix& camPos);

   /**
    * Get a debug node that contains a plane with the Camera's texture
    * applied to it.
    */
   OSG::NodePtr getDebugPlane() const;

   /**
    * Set the draw scale factor so we can generate correct visual aids
    */
   void setDrawScaleFactor(float scale);

   /**
    * Set the border width for the debug frame.
    */
   void setDebugFrameBorderWidth(float value);

   /**
    * Set the distance from the view the debug frame is.
    */
   void setDebugFrameDistance(float value);

   /**
    * Returns the root of the frame that surrounds what will be captured
    * in the recorder.
    */
   OSG::NodePtr getFrame() const;

   /** @name Signal Accessors */
   //@{
   typedef EncoderManager::basic_signal_t basic_signal_t;

   /**
    * Signal emitted when recording starts.
    *
    * @see vrkit::EncoderManager::encodingStarted()
    *
    * @since 0.45.2
    */
   signal::Proxy<basic_signal_t> recordingStarted()
   {
      return mVideoEncoder->encodingStarted();
   }

   /**
    * Signal emitted when recording is paused.
    *
    * @see vrkit::EncoderManager::encodingPaused()
    *
    * @since 0.45.2
    */
   signal::Proxy<basic_signal_t> recordingPaused()
   {
      return mVideoEncoder->encodingPaused();
   }

   /**
    * Signal emitted when recording resumes after being paused.
    *
    * @see vrkit::EncoderManager::encodingResumed()
    *
    * @since 0.45.2
    */
   signal::Proxy<basic_signal_t> recordingResumed()
   {
      return mVideoEncoder->encodingResumed();
   }

   /**
    * Signal emitted when recording stops.
    *
    * @see vrkit::EncoderManager::encodingStopped()
    *
    * @since 0.45.2
    */
   signal::Proxy<basic_signal_t> recordingStopped()
   {
      return mVideoEncoder->encodingStopped();
   }
   //@}

private:
   void generateDebugFrame();

   /**
    * Set the position of the camera.
    */
   void setCameraPos(const OSG::Matrix& camPos);

   CameraPtr            mCamera;         /**< Camera used for rendering. */
   EncoderManagerPtr    mVideoEncoder;
   OSG::ImagePtr        mStereoImageStorage; /**< Temp storage for stereo image concatenation. */
   OSG::TransformRefPtr mTransform;     /**< The location and orientation of the camera. */
   OSG::NodeRefPtr      mFrameRoot;     /**< The frame that surrounds the captured scene. */
   OSG::Real32          mEyeOffset;     /**< Interocular distance / 2 for stereo. */
   OSG::Real32          mBorderSize;    /**< The width of the frame geometry. */
   OSG::Real32          mFrameDist;     /**< The distance between the camera and the frame. */
   float                mDrawScale;   /**< Draw scale factor for the scene. */
};

}

}


#endif /* _VRKIT_VIDEO_RECORDER_H_ */
