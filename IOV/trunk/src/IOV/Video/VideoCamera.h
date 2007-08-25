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

#ifndef _INF_VIDEO_CAMERA_H_
#define _INF_VIDEO_CAMERA_H_

#include <IOV/Config.h>

#include <vector>
#include <boost/enable_shared_from_this.hpp>
#include <boost/signal.hpp>
#include <boost/signals/connection.hpp>

#include <OpenSG/OSGTransform.h>
#include <OpenSG/OSGWindow.h>
#include <OpenSG/OSGRenderAction.h>

#include <IOV/Util/SignalProxy.h>
#include <IOV/Video/CameraPtr.h>
#include <IOV/Video/VideoEncoder.h>
#include <IOV/Video/VideoCameraPtr.h>


namespace inf
{

/**
 * Records a scene with a camera and video encoder to a file.
 *
 * @since 0.43
 */
class IOV_CLASS_API VideoCamera : public boost::enable_shared_from_this<VideoCamera>
{
protected:
   VideoCamera();

public:
   static VideoCameraPtr create();

   virtual ~VideoCamera();

   /**
    * Initialize the Video Camera.
    */
   VideoCameraPtr init();

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
   void setFormat(const VideoEncoder::video_encoder_format_t& format);

   /*
    * Set the filename to record the video to.
    */
   void setFilename(const std::string& filename);

   /**
    * Set the field of view for the video camera.
    */
   void setFov(const OSG::Real32 fov);

   /**
    * Set the aspect ration for the video camera.
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
    * Set the near and far plane for the video camera.
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
    * Returns whether the video camera has started recording and has not
    * ended.
    */
   bool isRecording() const;

   /**
    * Returns whether the video camera is recording and paused.
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
    * Returns the root of the frame that surrounds what will be captured
    * in the video camera.
    */
   OSG::NodePtr getFrame() const;

   /** @name Signal Accessors */
   //@{
   typedef boost::signal<void ()> basic_signal_t;

   /**
    * Signal emitted when recording starts.
    *
    * @see inf::VideoEncoder::encodingStarted()
    *
    * @since 0.45.2
    */
   SignalProxy<basic_signal_t> recordingStarted()
   {
      return SignalProxy<basic_signal_t>(mRecordingStarted);
   }

   /**
    * Signal emitted when recording is paused.
    *
    * @see inf::VideoEncoder::encodingPaused()
    *
    * @since 0.45.2
    */
   SignalProxy<basic_signal_t> recordingPaused()
   {
      return SignalProxy<basic_signal_t>(mRecordingPaused);
   }

   /**
    * Signal emitted when recording resumes after being paused.
    *
    * @see inf::VideoEncoder::encodingResumed()
    *
    * @since 0.45.2
    */
   SignalProxy<basic_signal_t> recordingResumed()
   {
      return SignalProxy<basic_signal_t>(mRecordingResumed);
   }

   /**
    * Signal emitted when recording stops.
    *
    * @see inf::VideoEncoder::encodingStopped()
    *
    * @since 0.45.2
    */
   SignalProxy<basic_signal_t> recordingStopped()
   {
      return SignalProxy<basic_signal_t>(mRecordingStopped);
   }
   //@}

private:
   void generateDebugFrame();

   /**
    * Set the position of the camera.
    */
   void setCameraPos(const OSG::Matrix& camPos);

   /** @name Slots for inf::VideoEncoder Signals */
   //@{
   /**
    * Slot for the "encodingStarted" signal emitted by inf::VideoEncoder.
    * The "recordingStarted" signal is emitted by this object.
    *
    * @post \c mRecording is true. \c mPaused is false.
    *
    * @since 0.45.2
    */
   void encodingStarted();

   /**
    * Slot for the "encodingPaused" signal emitted by inf::VideoEncoder.
    * The "recordingPaused" signal is emitted by this object.
    *
    * @post \c mPaused is true.
    *
    * @since 0.45.2
    */
   void encodingPaused();

   /**
    * Slot for the "encodingResumed" signal emitted by inf::VideoEncoder.
    * The "recordingResumed" signal is emitted by this object.
    *
    * @post \c mPaused is false.
    *
    * @since 0.45.2
    */
   void encodingResumed();

   /**
    * Slot for the "encodingStopped" signal emitted by inf::VideoEncoder.
    * The "recordingStopped" signal is emitted by this object.
    *
    * @post \c mRecording and \c mPaused are both false.
    *
    * @since 0.45.2
    */
   void encodingStopped();
   //@}

   /** @name Signal Objects */
   //@{
   basic_signal_t mRecordingStarted;
   basic_signal_t mRecordingPaused;
   basic_signal_t mRecordingResumed;
   basic_signal_t mRecordingStopped;
   //@}

   std::vector<boost::signals::connection> mEncoderConnections;

   CameraPtr                            mCamera;         /**< Camera used for rendering. */
   VideoEncoderPtr                      mVideoEncoder;
   OSG::ImagePtr                        mStereoImageStorage; /**< Temp storage for stereo image concatenation. */
   OSG::RefPtr<OSG::TransformPtr>       mTransform;     /**< The location and orientation of the camera. */
   OSG::RefPtr<OSG::NodePtr>            mFrameRoot;     /**< The frame that surrounds the captured scene. */
   OSG::Real32                          mEyeOffset;     /**< Interocular distance / 2 for stereo. */
   OSG::Real32                          mBorderSize;    /**< The width of the frame geometry. */
   OSG::Real32                          mFrameDist;     /**< The distance between the camera and the frame. */
   bool                                 mStereo;        /**< If this camera should render/record in stereo. */
   bool                                 mRecording;        /**< Whether the camera is recording. */
   bool                                 mPaused;        /**< Whether the camera is paused. */
};

}

#endif /*_INF_FBO_VIDEO_CAMERA_H_*/
