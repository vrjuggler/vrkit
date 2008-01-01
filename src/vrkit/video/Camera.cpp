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

#include <OpenSG/OSGImage.h>

#include <vrkit/video/Camera.h>


namespace vrkit
{

namespace video
{

Camera::Camera()
   : mTransform(OSG::NullFC)
   , mLeftTexture(OSG::NullFC)
   , mRightTexture(OSG::NullFC)
   , mCurrentTexture(OSG::NullFC)
#if OSG_MAJOR_VERSION >= 2
   , mLeftTexEnv(OSG::NullFC)
   , mRightTexEnv(OSG::NullFC)
   , mCurrentTexEnv(OSG::NullFC)
#endif
   , mLeftImage(OSG::NullFC)
   , mRightImage(OSG::NullFC)
   , mCurrentImage(OSG::NullFC)
   , mCamera(OSG::NullFC)
   , mWidth(512)
   , mHeight(512)
{
   /* Do nothing. */ ;
}

Camera::~Camera()
{
   /* Do nothing. */ ;
}

CameraPtr Camera::init()
{
   // Create a transform to contain the location and orientation of the camera.
   mTransform = OSG::Transform::create();

   OSG::NodePtr beacon = OSG::Node::create();
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor be(beacon, OSG::Node::CoreFieldMask);
#endif
   beacon->setCore(mTransform);

   mLeftTexture = tex_chunk_t::create();
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor lte(mLeftTexture);
   mLeftTexture->setEnvMode(GL_MODULATE);
#else
   mLeftTexEnv = OSG::TextureEnvChunk::create();
   mLeftTexEnv->setEnvMode(GL_MODULATE);
#endif

   mRightTexture = tex_chunk_t::create();
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor rte(mRightTexture);
   mRightTexture->setEnvMode(GL_MODULATE);
#else
   mRightTexEnv = OSG::TextureEnvChunk::create();
   mRightTexEnv->setEnvMode(GL_MODULATE);
#endif

   mCurrentTexture = mLeftTexture;
#if OSG_MAJOR_VERSION >= 2
   mCurrentTexEnv = mLeftTexEnv;
#endif

   // setup camera
   mCamera = OSG::PerspectiveCamera::create();
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor ce(mCamera);
#endif
   mCamera->setFov(
#if OSG_MAJOR_VERSION < 2
      OSG::osgdegree2rad(60.0)
#else
      OSG::osgDegree2Rad(60.0)
#endif
   );
   mCamera->setNear(0.01);
   mCamera->setFar(10000);
   mCamera->setBeacon(beacon);

   mLeftImage = OSG::Image::create();
   mRightImage = OSG::Image::create();

   OSG::ImagePtr img;

   // Set up FBO textures.
   img = mLeftImage;
   mLeftTexture->setMinFilter(GL_LINEAR);
   mLeftTexture->setMagFilter(GL_LINEAR);
   mLeftTexture->setTarget(GL_TEXTURE_2D);
   mLeftTexture->setInternalFormat(GL_RGBA8);
   mLeftTexture->setImage(img);

   img = mRightImage;
   mRightTexture->setMinFilter(GL_LINEAR);
   mRightTexture->setMagFilter(GL_LINEAR);
   mRightTexture->setTarget(GL_TEXTURE_2D);
   mRightTexture->setInternalFormat(GL_RGBA8);
   mRightTexture->setImage(img);

   mCurrentImage = mLeftImage;

   return shared_from_this();
}

OSG::UInt32 Camera::getWidth() const
{
   return mWidth;
}

OSG::UInt32 Camera::getHeight() const
{
   return mHeight;
}

void Camera::renderLeftEye(render_action_t* ra)
{
   mCurrentTexture = mLeftTexture;
   mCurrentImage = mLeftImage;
   render(ra);
}

void Camera::renderRightEye(render_action_t* ra)
{
   mCurrentTexture = mRightTexture;
   mCurrentImage = mRightImage;
   render(ra);
}

void Camera::setSize(const OSG::UInt32 width, const OSG::UInt32 height)
{
   mWidth = width;
   mHeight = height;
}

void Camera::setPixelFormat(const OSG::Image::PixelFormat pixelFormat)
{
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor lie(mLeftImage);
   OSG::CPEditor rie(mRightImage);
#endif
   mLeftImage->set(pixelFormat, mWidth, mHeight);
   mRightImage->set(pixelFormat, mWidth, mHeight);
}

void Camera::setPosition(const OSG::Matrix& camPos)
{
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor te(mTransform, OSG::Transform::MatrixFieldMask);
#endif
   mTransform->setMatrix(camPos);
}

void Camera::setAspect(const OSG::Real32 aspect)
{
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor ce(mCamera, OSG::PerspectiveCamera::AspectFieldMask);
#endif
   mCamera->setAspect(aspect);
}

void Camera::setFov(const OSG::Real32 fov)
{
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor ce(mCamera, OSG::PerspectiveCamera::FovFieldMask);
#endif
   mCamera->setFov(fov);
}

void Camera::setNearFar(const OSG::Real32 nearVal,
                                const OSG::Real32 farVal)
{
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor ce(mCamera,
                    OSG::PerspectiveCamera::NearFieldMask |
                       OSG::PerspectiveCamera::FarFieldMask);
#endif
   mCamera->setNear(nearVal);
   mCamera->setFar(farVal);
}

void Camera::setWindow(OSG::WindowPtr window)
{
   mWindow = window;
}

Camera::tex_chunk_ptr_t Camera::getLeftTexture() const
{
   return mLeftTexture;
}

Camera::tex_chunk_ptr_t Camera::getRightTexture() const
{
   return mRightTexture;
}

#if OSG_MAJOR_VERSION >= 2
OSG::TextureEnvChunkPtr Camera::getLeftTextureEnv() const
{
   return mLeftTexEnv;
}

OSG::TextureEnvChunkPtr Camera::getRightTextureEnv() const
{
   return mRightTexEnv;
}
#endif

OSG::ImagePtr Camera::getLeftEyeImage() const
{
   return mLeftImage;
}

OSG::ImagePtr Camera::getRightEyeImage() const
{
   return mRightImage;
}

OSG::Real32 Camera::getFov() const
{
   return mCamera->getFov();
}

OSG::Real32 Camera::getAspect() const
{
   return mCamera->getAspect();
}

}

}
