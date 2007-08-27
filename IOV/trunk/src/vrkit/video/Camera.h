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

#ifndef _VRKIT_VIDEO_CAMERA_H_
#define _VRKIT_VIDEO_CAMERA_H_

#include <vrkit/Config.h>

#include <boost/enable_shared_from_this.hpp>

#include <OpenSG/OSGImage.h>
#include <OpenSG/OSGPerspectiveCamera.h>
#include <OpenSG/OSGRenderAction.h>
#include <OpenSG/OSGTextureChunk.h>
#include <OpenSG/OSGTransform.h>
#include <OpenSG/OSGWindow.h>

#include <vrkit/video/CameraPtr.h>


namespace vrkit
{

namespace video
{

/**
 * Base Camera class.
 *
 * @note This class was moved into vrkit::video namespace in version 0.47.
 *
 * @since 0.43
 */
class VRKIT_CLASS_API Camera : public boost::enable_shared_from_this<Camera>
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

   void setPixelFormat(const OSG::Image::PixelFormat pixelFormat);

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

   OSG::TransformRefPtr                 mTransform;     /**< The location and orientation of the camera. */
   OSG::TextureChunkPtr                 mLeftTexture;   /**< Texture that the FBO renders into. */
   OSG::TextureChunkPtr                 mRightTexture;  /**< Texture that the FBO renders into. */
   OSG::TextureChunkPtr                 mCurrentTexture;  /**< Texture that the FBO renders into. */
   OSG::ImagePtr                        mLeftImage;
   OSG::ImagePtr                        mRightImage;
   OSG::ImagePtr                        mCurrentImage;
   OSG::PerspectiveCameraPtr            mCamera;       /**< Perspective camera for the FBO. */
   OSG::UInt32                          mWidth;        /**< Width of the FBO. */
   OSG::UInt32                          mHeight;       /**< Height of the FBO. */
   OSG::WindowPtr                       mWindow;
};

}

}


#endif /* _VRKIT_VIDEO_CAMERA_H_ */
