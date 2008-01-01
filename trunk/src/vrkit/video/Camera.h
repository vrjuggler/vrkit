// vrkit is (C) Copyright 2005-2008
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
#if OSG_MAJOR_VERSION < 2
#  include <OpenSG/OSGTextureChunk.h>
#else
#  include <OpenSG/OSGTextureObjChunk.h>
#  include <OpenSG/OSGTextureEnvChunk.h>
#endif
#include <OpenSG/OSGTransform.h>
#include <OpenSG/OSGWindow.h>

#include <vrkit/video/CameraPtr.h>


OSG_BEGIN_NAMESPACE
#if OSG_MAJOR_VERSION < 2
class RenderAction;
#else
class RenderTraversalAction;
#endif
OSG_END_NAMESPACE

namespace vrkit
{

namespace video
{

/** \class Camera Camera.h vrkit/video/Camera.h
 *
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

   /**
    * @name Render Action Type
    *
    * OpenSG 1.8/2.0 compatibility typedefs. These are mainly for internal
    * use, but they can be used in user-level code to help bridge the
    * differences between OpenSG 1.8 and 2.0.
    *
    * @see renderLeftEye()
    * @see renderRightEye()
    * @see render()
    *
    * @since 0.51.0
    */
   //@{
#if OSG_MAJOR_VERSION < 2
   typedef OSG::RenderAction render_action_t;
#else
   typedef OSG::RenderTraversalAction render_action_t;
#endif
   //@}

   virtual void renderLeftEye(render_action_t* ra);

   virtual void renderRightEye(render_action_t* ra);

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

   /**
    * @name Texture Chunk Types
    *
    * OpenSG 1.8/2.0 compatibility typedefs. These are mainly for internal
    * use, but they can be used in user-level code to help bridge the
    * differences between OpenSG 1.8 and 2.0.
    *
    * @see getLeftTexture()
    * @see getRightTexture()
    *
    * @since 0.51.0
    */
   //@{
#if OSG_MAJOR_VERSION < 2
   typedef OSG::TextureChunk            tex_chunk_t;
   typedef OSG::TextureChunkPtr         tex_chunk_ptr_t;
#else
   typedef OSG::TextureObjChunk         tex_chunk_t;
   typedef OSG::TextureObjChunkPtr      tex_chunk_ptr_t;
#endif
   typedef OSG::RefPtr<tex_chunk_ptr_t> tex_chunk_ref_ptr_t;
   //@}

   virtual tex_chunk_ptr_t getLeftTexture() const;
   virtual tex_chunk_ptr_t getRightTexture() const;

#if OSG_MAJOR_VERSION >= 2
   /**
    * @since 0.51.0
    */
   virtual OSG::TextureEnvChunkPtr getLeftTextureEnv() const;

   /**
    * @since 0.51.0
    */
   virtual OSG::TextureEnvChunkPtr getRightTextureEnv() const;
#endif

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

   virtual void render(render_action_t* ra) = 0;

   OSG::TransformRefPtr       mTransform;     /**< The location and orientation of the camera. */
   tex_chunk_ref_ptr_t        mLeftTexture;   /**< Texture that the FBO renders into. */
   tex_chunk_ref_ptr_t        mRightTexture;  /**< Texture that the FBO renders into. */
   tex_chunk_ref_ptr_t        mCurrentTexture;  /**< Texture that the FBO renders into. */
#if OSG_MAJOR_VERSION >= 2
   OSG::TextureEnvChunkRefPtr mLeftTexEnv;
   OSG::TextureEnvChunkRefPtr mRightTexEnv;
   OSG::TextureEnvChunkRefPtr mCurrentTexEnv;
#endif
   OSG::ImageRefPtr           mLeftImage;
   OSG::ImageRefPtr           mRightImage;
   OSG::ImageRefPtr           mCurrentImage;
   OSG::PerspectiveCameraPtr  mCamera;       /**< Perspective camera for the FBO. */
   OSG::UInt32                mWidth;        /**< Width of the FBO. */
   OSG::UInt32                mHeight;       /**< Height of the FBO. */
   OSG::WindowPtr             mWindow;
};

}

}


#endif /* _VRKIT_VIDEO_CAMERA_H_ */
