// Copyright (C) Infiscape Corporation 2005-2007

#include <OpenSG/OSGFrustumVolume.h>
#include <OpenSG/OSGGeometry.h>
#include <OpenSG/OSGGeoPropPtrs.h>
#include <OpenSG/OSGImage.h>
#include <OpenSG/OSGMergeGraphOp.h>
#include <OpenSG/OSGGraphOpSeq.h>
#include <OpenSG/OSGSimpleGeometry.h>
#include <OpenSG/OSGSimpleMaterial.h>
#include <OpenSG/OSGSolidBackground.h>

#include <vrj/Display/Projection.h>

#include <IOV/Video/VideoEncoder.h>
#include <IOV/Video/Camera.h>

namespace inf
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
{;}

Camera::~Camera()
{
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

   // Set the correct size of FBO.
   // This also generates the frame geometry around the captured scene.
   inf::Camera::setSize(mWidth, mHeight);

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

   // Resize the debug texture.
   OSG::beginEditCP(mLeftImage);
      mLeftImage->set(OSG::Image::OSG_RGBA_PF, width, height);
   OSG::endEditCP(mLeftImage);

   OSG::beginEditCP(mRightImage);
      mRightImage->set(OSG::Image::OSG_RGBA_PF, width, height);
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
