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
   OSG::beginEditCP(beacon);
      beacon->setCore(mTransform);
   OSG::endEditCP(beacon);

   // Create the FBO textures.
   mLeftTexture = OSG::TextureChunk::create();
   OSG::beginEditCP(mLeftTexture);
      mLeftTexture->setEnvMode(GL_MODULATE);
   OSG::endEditCP(mLeftTexture);

   mRightTexture = OSG::TextureChunk::create();
   OSG::beginEditCP(mRightTexture);
      mRightTexture->setEnvMode(GL_MODULATE);
   OSG::endEditCP(mRightTexture);

   mCurrentTexture = mLeftTexture;

   // setup camera
   mCamera = OSG::PerspectiveCamera::create();
   OSG::beginEditCP(mCamera);
      mCamera->setFov(OSG::osgdegree2rad(60.0));
      mCamera->setNear(0.01);
      mCamera->setFar(10000);
      mCamera->setBeacon(beacon);
   OSG::endEditCP(mCamera);

   mLeftImage = OSG::Image::create();
   mRightImage = OSG::Image::create();

   // Setup FBO textures.
   OSG::beginEditCP(mLeftTexture);
      mLeftTexture->setMinFilter(GL_LINEAR);
      mLeftTexture->setMagFilter(GL_LINEAR);
      mLeftTexture->setTarget(GL_TEXTURE_2D);
      mLeftTexture->setInternalFormat(GL_RGBA8);
      mLeftTexture->setImage(mLeftImage);
   OSG::endEditCP(mLeftTexture);

   OSG::beginEditCP(mRightTexture);
      mRightTexture->setMinFilter(GL_LINEAR);
      mRightTexture->setMagFilter(GL_LINEAR);
      mRightTexture->setTarget(GL_TEXTURE_2D);
      mRightTexture->setInternalFormat(GL_RGBA8);
      mRightTexture->setImage(mRightImage);
   OSG::endEditCP(mRightTexture);

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

void Camera::renderLeftEye(OSG::RenderAction* ra)
{
   mCurrentTexture = mLeftTexture;
   mCurrentImage = mLeftImage;
   render(ra);
}

void Camera::renderRightEye(OSG::RenderAction* ra)
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
   OSG::beginEditCP(mLeftImage);
      mLeftImage->set(pixelFormat, mWidth, mHeight);
   OSG::endEditCP(mLeftImage);

   OSG::beginEditCP(mRightImage);
      mRightImage->set(pixelFormat, mWidth, mHeight);
   OSG::endEditCP(mRightImage);
}

void Camera::setPosition(const OSG::Matrix& camPos)
{
   OSG::beginEditCP(mTransform, OSG::Transform::MatrixFieldMask);
      mTransform->setMatrix(camPos);
   OSG::endEditCP(mTransform, OSG::Transform::MatrixFieldMask);
}

void Camera::setAspect(const OSG::Real32 aspect)
{
   OSG::beginEditCP(mCamera);
      mCamera->setAspect(aspect);
   OSG::endEditCP(mCamera);
}

void Camera::setFov(const OSG::Real32 fov)
{
   OSG::beginEditCP(mCamera);
      mCamera->setFov(fov);
   OSG::endEditCP(mCamera);
}

void Camera::setNearFar(const OSG::Real32 nearVal,
                                const OSG::Real32 farVal)
{
   OSG::beginEditCP(mCamera);
      mCamera->setNear(nearVal);
      mCamera->setFar(farVal);
   OSG::endEditCP(mCamera);
}

void Camera::setWindow(OSG::WindowPtr window)
{
   mWindow = window;
}

OSG::TextureChunkPtr Camera::getLeftTexture() const
{
   return mLeftTexture;
}

OSG::TextureChunkPtr Camera::getRightTexture() const
{
   return mRightTexture;
}

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
