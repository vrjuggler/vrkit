// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_VIDEO_CAMERA_H_
#define _INF_VIDEO_CAMERA_H_

#include <IOV/Config.h>

#include <boost/enable_shared_from_this.hpp>

#include <OpenSG/OSGTransform.h>
#include <OpenSG/OSGWindow.h>

#include <IOV/Video/CameraPtr.h>
#include <IOV/Video/VideoEncoderPtr.h>
#include <IOV/Video/VideoCameraPtr.h>

namespace inf
{

/**
 * Records a scene with a camera and video encoder to a file.
 *
 * @since 0.42
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

   void setTravMask(const UInt32 value);

   void setSceneRoot(OSG::NodePtr root);

   void setFilename(const std::string& filename);

   void setFramesPerSecond(const OSG::UInt32 framesPerSecond);

   void setCodec(const std::string& codec);

   /**
    * Returns the current set of available codes.
    */
   const std::set<std::string>& getAvailableCodecs() const;

   void setStereo(const bool stereo);

   void setInterocularDistance(const OSG::Real32 interocular);

   void startRecording();

   void pause();

   void resume();

   void endRecording();

   bool isRecording()
   {
      return mRecording;
   }

   bool isPaused()
   {
      return mPaused;
   }

   void render(OSG::RenderAction* ra, const OSG::Matrix camPos);

   /**
    * Set the field of view for the video camera.
    */
   void setFov(const OSG::Real32 fov);

   /**
    * Set the near and far plane for the video camera.
    */
   void setNearFar(const OSG::Real32 nearVal, const OSG::Real32 farVal);

   /**
    * Set the size of a video frame.
    */
   void setSize(const OSG::UInt32 width, const OSG::UInt32 height);

   /**
    * Get a debug node that contains a plane with the FBO texture
    * applied to it.
    */
   OSG::NodePtr getDebugPlane() const;

   /**
    * Returns the root of the frame that surrounds what will be captured
    * in the video camera.
    */
   OSG::NodePtr getFrame() const
   {
      return mFrameRoot;
   }

private:
   void generateDebugFrame();

   /**
    * Set the position of the camera.
    */
   void setCameraPos(const OSG::Matrix camPos);

private:
   CameraPtr		                mCamera;         /**< Camera used for rendering. */
   VideoEncoderPtr			mVideoEncoder;
   OSG::RefPtr<OSG::TransformPtr>       mTransform;     /**< The location and orientation of the camera. */
   OSG::RefPtr<OSG::NodePtr>            mFrameRoot;     /**< The frame that surrounds the captured scene. */
   OSG::Real32                          mEyeOffset;	/**< Interocular distance / 2 for stereo. */
   OSG::Real32                          mBorderSize;    /**< The width of the frame geometry. */
   OSG::Real32                          mFrameDist;     /**< The distance between the camera and the frame. */
   bool                                 mStereo;        /**< If this camera should render/record in stereo. */
   bool                                 mRecording;        /**< Whether the camera is recording. */
   bool                                 mPaused;        /**< Whether the camera is paused. */
};

}

#endif /*_INF_FBO_VIDEO_CAMERA_H_*/
