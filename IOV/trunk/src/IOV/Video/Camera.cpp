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
   , mFov(60.0)
   , mBorderSize(2.0)
   , mFrameDist(100.0)
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

   // Create the frame root.
   mFrameRoot = OSG::Node::create();
   OSG::beginEditCP(mFrameRoot);
      mFrameRoot->setCore(mTransform);
   OSG::endEditCP(mFrameRoot);

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
      // If fov is in degrees, convert to radians first.
      if (mFov > OSG::Pi)
      {
         mCamera->setFov(OSG::osgdegree2rad(mFov));
      }
      else
      {
         mCamera->setFov(mFov);
      }
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

   // We already compensated aspect ratio with the texture/fbo sizes
   OSG::beginEditCP(mCamera);
      mCamera->setAspect(width/height);
   OSG::endEditCP(mCamera);

   generateDebugFrame();
}

void Camera::setPosition(const OSG::Matrix& camPos)
{
   OSG::beginEditCP(mTransform, OSG::Transform::MatrixFieldMask);
      mTransform->setMatrix(camPos);
   OSG::endEditCP(mTransform, OSG::Transform::MatrixFieldMask);
}

void Camera::setFov(const OSG::Real32 fov)
{
   OSG::beginEditCP(mCamera);
      mCamera->setFov(fov);
   OSG::endEditCP(mCamera);

   generateDebugFrame();
}

void Camera::setNearFar(const OSG::Real32 nearVal,
                                const OSG::Real32 farVal)
{
   OSG::beginEditCP(mCamera);
      mCamera->setNear(nearVal);
      mCamera->setFar(farVal);
   OSG::endEditCP(mCamera);
}

OSG::NodePtr Camera::getDebugPlane() const
{
   // Create material for left eye.
   OSG::SimpleMaterialPtr left_mat = OSG::SimpleMaterial::create();
   OSG::beginEditCP(left_mat);
      left_mat->addChunk(mLeftTexture);
      left_mat->setSpecular(OSG::Color3f(0.7f, 0.7f, 0.7f));
      left_mat->setDiffuse(OSG::Color3f(0.22f, 0.2f, 0.2f));
   OSG::endEditCP(left_mat);

   // Create material for right eye.
   OSG::SimpleMaterialPtr right_mat = OSG::SimpleMaterial::create();
   OSG::beginEditCP(right_mat);
      right_mat->addChunk(mRightTexture);
      right_mat->setSpecular(OSG::Color3f(0.7f, 0.7f, 0.7f));
      right_mat->setDiffuse(OSG::Color3f(0.22f, 0.2f, 0.2f));
   OSG::endEditCP(right_mat);

   // Create geometry for left eye.
   OSG::GeometryPtr left_geom = OSG::makePlaneGeo(5, 5, 2, 2);
   OSG::NodePtr left_node = OSG::Node::create();
   OSG::beginEditCP(left_geom);
      left_geom->setMaterial(left_mat);
   OSG::endEditCP(left_geom);
   OSG::beginEditCP(left_node);
      left_node->setCore(left_geom);
   OSG::endEditCP(left_node);

   // Create geometry for right eye.
   OSG::GeometryPtr right_geom = OSG::makePlaneGeo(5, 5, 2, 2);
   OSG::NodePtr right_node = OSG::Node::create();
   OSG::beginEditCP(right_geom);
      right_geom->setMaterial(right_mat);
   OSG::endEditCP(right_geom);
   OSG::beginEditCP(right_node);
      right_node->setCore(right_geom);
   OSG::endEditCP(right_node);

   // Create the xforms for each eye's geometry.
   OSG::Matrix leftm, rightm;
   leftm.setTranslate(-2.5, 2.5, 0.0);
   rightm.setTranslate(2.5, 2.5, 0.0);

   // Create the transform nodes for each eye's geometry.
   OSG::TransformNodePtr left_xform = OSG::TransformNodePtr::create();
   OSG::TransformNodePtr right_xform = OSG::TransformNodePtr::create();
   OSG::beginEditCP(left_xform);
   OSG::beginEditCP(right_xform);
      left_xform->setMatrix(leftm);
      right_xform->setMatrix(rightm);
      left_xform.node()->addChild(left_node);
      right_xform.node()->addChild(right_node);
   OSG::endEditCP(left_xform);
   OSG::endEditCP(right_xform);

   OSG::GroupPtr group = OSG::Group::create();
   OSG::NodePtr group_node = OSG::Node::create();
   OSG::beginEditCP(group_node);
      group_node->setCore(group);
      group_node->addChild(left_xform.node());
      group_node->addChild(right_xform.node());
   OSG::endEditCP(group_node);
   return group_node;
}

void Camera::setWindow(OSG::WindowPtr window)
{
   mWindow = window;
}

OSG::NodePtr Camera::getFrame() const
{
   return mFrameRoot;
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

// XXX: This has not been updated to behave correctly in stereo mode.
void Camera::generateDebugFrame()
{
   // The size of the internal frame.
   OSG::Real32 frame_height = 2.0 * (OSG::osgtan(mCamera->getFov()/2.0) * mFrameDist);
   OSG::Real32 frame_width = frame_height * mCamera->getAspect();

   // =============
   // |           |
   // |           |
   // |           |
   // |           |
   // =============

   // Top and bottom lines need to be long enough to go past left and
   // right sides.
   OSG::Real32 real_frame_width = frame_width + (2.0*mBorderSize);

   // Create the geometry nodes.
   OSG::GeometryNodePtr top(OSG::makeBox(real_frame_width, mBorderSize, mBorderSize, 2, 2, 2));
   OSG::GeometryNodePtr bottom(OSG::makeBox(real_frame_width, mBorderSize, mBorderSize, 2, 2, 2));
   OSG::GeometryNodePtr left(OSG::makeBox(mBorderSize, frame_height, mBorderSize, 2, 2, 2));
   OSG::GeometryNodePtr right(OSG::makeBox(mBorderSize, frame_height, mBorderSize, 2, 2, 2));

   OSG::TransformNodePtr top_xform = OSG::TransformNodePtr::create();
   OSG::TransformNodePtr bottom_xform = OSG::TransformNodePtr::create();
   OSG::TransformNodePtr left_xform = OSG::TransformNodePtr::create();
   OSG::TransformNodePtr right_xform = OSG::TransformNodePtr::create();

   // Create material for the frame.
   OSG::SimpleMaterialPtr mat = OSG::SimpleMaterial::create();
   OSG::beginEditCP(mat);
      //mat->setLit(false);
      //mat->setDiffuse(OSG::Color3f(1.0f, 0.0f, 0.0f));
      //mat->setAmbient(OSG::Color3f(0.2f, 0.2f, 0.2f));
      mat->setDiffuse(OSG::Color3f(0.8f, 0.0f, 0.0f));
      //mat->setSpecular(OSG::Color3f(0.2f, 0.2f, 0.2f));
      //mat->setShininess(2);
   OSG::endEditCP(mat);

   // Set the material for all parts of the frame.
   OSG::beginEditCP(top);
   OSG::beginEditCP(bottom);
   OSG::beginEditCP(left);
   OSG::beginEditCP(right);
      top->setMaterial(mat);
      bottom->setMaterial(mat);
      left->setMaterial(mat);
      right->setMaterial(mat);
   OSG::endEditCP(top);
   OSG::endEditCP(bottom);
   OSG::endEditCP(left);
   OSG::endEditCP(right);

   // We need to pull the frame in half the border size to ensure that we don't
   // see it.
   OSG::Real32 zoffset = -mFrameDist + (mBorderSize/2.0);
   OSG::Real32 xoffset = (frame_width/2.0) + (mBorderSize/2.0);
   OSG::Real32 yoffset = (frame_height/2.0) + (mBorderSize/2.0);

   // Create the xforms for each box.
   OSG::Matrix topm, bottomm, leftm, rightm;
   topm.setTranslate(0.0, yoffset, zoffset);
   bottomm.setTranslate(0.0, -yoffset, zoffset);
   leftm.setTranslate(-xoffset, 0.0, zoffset);
   rightm.setTranslate(xoffset, 0.0, zoffset);


   OSG::beginEditCP(top_xform);
   OSG::beginEditCP(bottom_xform);
   OSG::beginEditCP(left_xform);
   OSG::beginEditCP(right_xform);
      top_xform->setMatrix(topm);
      bottom_xform->setMatrix(bottomm);
      left_xform->setMatrix(leftm);
      right_xform->setMatrix(rightm);

      top_xform.node()->addChild(top.node());
      bottom_xform.node()->addChild(bottom.node());
      left_xform.node()->addChild(left.node());
      right_xform.node()->addChild(right.node());
   OSG::endEditCP(top_xform);
   OSG::endEditCP(bottom_xform);
   OSG::endEditCP(left_xform);
   OSG::endEditCP(right_xform);

   // Create a node that will contain all geometry after the the merge op.
   OSG::NodePtr frame = OSG::Node::create();
   OSG::GroupPtr frame_group = OSG::Group::create();
   OSG::beginEditCP(frame);
      frame->setCore(frame_group);
      frame->addChild(top_xform.node());
      frame->addChild(bottom_xform.node());
      frame->addChild(left_xform.node());
      frame->addChild(right_xform.node());
   OSG::endEditCP(frame);

   // Merge all geometry into one node.
   OSG::MergeGraphOp merge;
   merge.traverse(frame);

   // Remove old geometry and add new.
   OSG::beginEditCP(mFrameRoot);
      while (mFrameRoot->getNChildren() > 0)
      {
         mFrameRoot->subChild(0);
      }
      mFrameRoot->addChild(frame);
   OSG::endEditCP(mFrameRoot);
}

}
