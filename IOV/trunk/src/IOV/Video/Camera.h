// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_CAMERA_H_
#define _INF_CAMERA_H_

#include <IOV/Config.h>

#include <boost/enable_shared_from_this.hpp>

#include <OpenSG/OSGImage.h>
#include <OpenSG/OSGPerspectiveCamera.h>
#include <OpenSG/OSGRenderAction.h>
#include <OpenSG/OSGTextureChunk.h>
#include <OpenSG/OSGTransform.h>
#include <OpenSG/OSGWindow.h>

#include <IOV/Video/CameraPtr.h>
#include <IOV/Video/VideoEncoderPtr.h>

namespace inf
{

/**
 * Base Camera class.
 *
 * @since 0.43
 */
class IOV_CLASS_API Camera : public boost::enable_shared_from_this<inf::Camera>
{

protected:
   Camera();

public:

   virtual ~Camera();

   /**
    * Initialize the camera.
    */
   virtual CameraPtr init();

   /**
    * Called from the Viewer's context init so that we can set the
    * correct window.
    */

   virtual void setSceneRoot(OSG::NodePtr root) = 0;

   virtual void renderLeftEye(OSG::RenderAction* ra);

   virtual void renderRightEye(OSG::RenderAction* ra);

   /**
    * Set the field of view for the camera.
    */
   void setFov(const OSG::Real32 fov);

   /**
    * Set aspect ratio for the camera.
    */
   void setAspect(const OSG::Real32 aspect);

   /**
    * Set the near and far plane for the camera.
    *
    */
   virtual void setNearFar(const OSG::Real32 nearVal, const OSG::Real32 farVal);

   /**
    * Set the size of a frame.
    */
   virtual void setSize(const OSG::UInt32 width, const OSG::UInt32 height);

   virtual OSG::UInt32 getWidth() const;

   virtual OSG::UInt32 getHeight() const;

   virtual void setWindow(OSG::WindowPtr window);

   virtual OSG::TextureChunkPtr getLeftTexture() const;
   virtual OSG::TextureChunkPtr getRightTexture() const;

   virtual OSG::ImagePtr getLeftEyeImage() const;
   virtual OSG::ImagePtr getRightEyeImage() const;

   virtual OSG::Real32 getFov() const;

   virtual OSG::Real32 getAspect() const;

   virtual void setTravMask(const OSG::UInt32 value) = 0;

   /**
    * Set the position of the camera.
    */
   virtual void setPosition(const OSG::Matrix& camPos);

protected:

   virtual void render(OSG::RenderAction* ra) = 0;



   OSG::RefPtr<OSG::TransformPtr>       mTransform;     /**< The location and orientation of the camera. */
   OSG::TextureChunkPtr                 mLeftTexture;   /**< Texture that the FBO renders into. */
   OSG::TextureChunkPtr                 mRightTexture;  /**< Texture that the FBO renders into. */
   OSG::TextureChunkPtr                 mCurrentTexture;  /**< Texture that the FBO renders into. */
   OSG::ImagePtr			mLeftImage;
   OSG::ImagePtr			mRightImage;
   OSG::ImagePtr			mCurrentImage;
   OSG::PerspectiveCameraPtr            mCamera;        /**< Perspective camera for the FBO. */
   OSG::UInt32				mWidth;		/**< Width of the FBO. */
   OSG::UInt32				mHeight;	/**< Height of the FBO. */
   OSG::WindowPtr			mWindow;
};

}

#endif /*_INF_CAMERA_H_*/
