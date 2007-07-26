// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_FBO_VIDEO_CAMERA_H_
#define _INF_FBO_VIDEO_CAMERA_H_

#include <IOV/Config.h>

#include <boost/enable_shared_from_this.hpp>

#include <OpenSG/OSGFBOViewport.h>
#include <OpenSG/OSGPerspectiveCamera.h>
#include <OpenSG/OSGTransform.h>
#include <OpenSG/OSGTextureChunk.h>
#include <OpenSG/OSGWindow.h>

#include <IOV/Video/FboVideoCameraPtr.h>

namespace inf
{

/**
 * Renders a scene from a perspective camera into a FBO.
 *
 * @since 0.37
 */
class IOV_CLASS_API FboVideoCamera : public boost::enable_shared_from_this<FboVideoCamera>
{
protected:
   FboVideoCamera()
      : mFboVP(OSG::NullFC)
      , mTransform(OSG::NullFC)
      , mLeftTexture(OSG::NullFC)
      , mRightTexture(OSG::NullFC)
      , mFboCam(OSG::NullFC)
      , mBorderSize(2.0)
      , mFrameDist(100.0)
   {;}

public:
   static FboVideoCameraPtr create();

   virtual ~FboVideoCamera();

   /**
    * Initialize the FBO camera.
    */
   FboVideoCameraPtr init(const OSG::UInt32 width = 512, const OSG::UInt32 height = 512,
                          const OSG::Real32 fov = 60.0, const OSG::Real32 borderSize = 2.0,
                          const OSG::Real32 frameDist = 100.0);

   /**
    * Called from the Viewer's context init so that we can set the
    * correct window.
    */
   void contextInit(OSG::WindowPtr gwin);

   /**
    * Returns the FBOViewport.
    */
   OSG::FBOViewportPtr getFboViewport() const
   {
      return mFboVP;
   }

   /**
    * Set the position of the camera.
    */
   void setCameraPos(const OSG::Matrix camPos, const OSG::Real32 interocular = 0.0f,
                     const OSG::UInt32 currentEye = 0);

   /**
    * Set the field of view for the camera.
    */
   void setFov(const OSG::Real32 fov)
   {
      mFboCam->setFov(fov);
      generateFrame();
   }

   /**
    * Set the aspect ratio (w/h) for the camera.
    */
   /* XXX: Doesn't make sense unless the user want's to make a
    *      video that has the incorrect aspect for the given
    *      width and height.
   void setAspect(const OSG::Real32 fov)
   {
      mFboCam->setAspect(fov);
      generateFrame();
   }
   */

   /**
    * Set the size of the FBO you want to use.
    */
   void setSize(const OSG::UInt32 width, const OSG::UInt32 height);

   /**
    * Get a debug node that contains a plane with the FBO texture
    * applied to it.
    */
   OSG::NodePtr getDebugPlane() const;

   /**
    * Returns the root of the frame that surrounds what will be captured
    * in the FBOViewport.
    */
   OSG::NodePtr getFrame() const
   {
      return mFrameRoot;
   }

private:
   void generateFrame();

private:
   OSG::FBOViewportPtr                  mFboVP;         /**< FBOViewport that we use to render to an FBO. */
   OSG::RefPtr<OSG::TransformPtr>       mTransform;     /**< The location and orientation of the camera. */
   OSG::RefPtr<OSG::NodePtr>            mFrameRoot;     /**< The frame that surrounds the captured scene. */
   OSG::TextureChunkPtr                 mLeftTexture;   /**< Texture that the FBO renders into. */
   OSG::TextureChunkPtr                 mRightTexture;  /**< Texture that the FBO renders into. */
   OSG::PerspectiveCameraPtr            mFboCam;        /**< Perspective camera for the FBO. */
   OSG::Real32                          mBorderSize;    /**< The width of the frame geometry. */
   OSG::Real32                          mFrameDist;     /**< The distance between the camera and the frame. */
};

}

#endif /*_INF_FBO_VIDEO_CAMERA_H_*/
