// Copyright (C) Infiscape Corporation 2005-2007

#include <IOV/Video/FboVideoCamera.h>

#include <OpenSG/OSGFrustumVolume.h>
#include <OpenSG/OSGGeometry.h>
#include <OpenSG/OSGGeoPropPtrs.h>
#include <OpenSG/OSGImage.h>
#include <OpenSG/OSGMergeGraphOp.h>
#include <OpenSG/OSGGraphOpSeq.h>
#include <OpenSG/OSGSimpleGeometry.h>
#include <OpenSG/OSGSimpleMaterial.h>
#include <OpenSG/OSGSolidBackground.h>

namespace inf
{

FboVideoCameraPtr FboVideoCamera::create()
{
   return FboVideoCameraPtr(new FboVideoCamera);
}

FboVideoCamera::~FboVideoCamera()
{
}

void FboVideoCamera::contextInit(OSG::WindowPtr gwin)
{
   mFboVP->setParent(gwin);
}

FboVideoCameraPtr FboVideoCamera::init(const OSG::UInt32 width, const OSG::UInt32 height, const OSG::Real32 fov,
                                       const OSG::Real32 borderSize, const OSG::Real32 frameDist)
{
   mBorderSize = borderSize;
   mFrameDist = frameDist;

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

   // Where should we attach the FBO camera beacon.
   //OSG::beginEditCP(fboRoot);
      //fboRoot->addChild(beacon);
   //OSG::endEditCP(fboRoot);

   // Create the FBO texture.
   mFboTexture = OSG::TextureChunk::create();
   OSG::beginEditCP(mFboTexture);
      mFboTexture->setEnvMode(GL_MODULATE);
   OSG::endEditCP(mFboTexture);

   // setup camera
   mFboCam = OSG::PerspectiveCamera::create();
   OSG::beginEditCP(mFboCam);
      // If fov is in degrees, convert to radians first.
      if (fov > OSG::Pi)
      {
         mFboCam->setFov(OSG::osgdegree2rad(fov));
      }
      else
      {
         mFboCam->setFov(fov);
      }
      mFboCam->setNear(0.01);
      mFboCam->setFar(10000);
      mFboCam->setBeacon(beacon);
   OSG::endEditCP(mFboCam);

   // Setup FBO
   OSG::beginEditCP(mFboTexture);
      mFboTexture->setMinFilter(GL_LINEAR);
      mFboTexture->setMagFilter(GL_LINEAR);
      mFboTexture->setTarget(GL_TEXTURE_2D);
      mFboTexture->setInternalFormat(GL_RGBA8);
   OSG::endEditCP(mFboTexture);
   
   OSG::SolidBackgroundPtr bg = OSG::SolidBackground::create();
   OSG::beginEditCP(bg);
      bg->setColor(OSG::Color3f(0, 0, 0));
   OSG::endEditCP(bg);

   // create FBOViewport
   mFboVP = OSG::FBOViewport::create();
   OSG::beginEditCP(mFboVP);
      mFboVP->setBackground(bg);
      mFboVP->setCamera(mFboCam);
      mFboVP->getTextures().push_back(mFboTexture);
   OSG::endEditCP(mFboVP);

   // Set the correct size of FBO.
   // This also generates the frame geometry around the captured scene.
   setSize(width, height);

   return shared_from_this();
}

void FboVideoCamera::setSize(const OSG::UInt32 width, const OSG::UInt32 height)
{
   // Resize the viewport.
   OSG::beginEditCP(mFboVP);
      mFboVP->setSize(0, 0, width - 1, height - 1);
      mFboVP->setStorageWidth(width);
      mFboVP->setStorageHeight(height);
   OSG::endEditCP(mFboVP);

   // Resize the debug texture.
   OSG::ImagePtr img = OSG::Image::create();
   OSG::beginEditCP(img);
      img->set(OSG::Image::OSG_RGBA_PF, width, height);
   OSG::endEditCP(img);
   OSG::beginEditCP(mFboTexture);
      mFboTexture->setImage(img);
   OSG::endEditCP(mFboTexture);

   //setAspect(width/ height);
   // We already compensated aspect ratio with the texture/fbo sizes
   mFboCam->setAspect(width/height);
   generateFrame();
}

void FboVideoCamera::setCameraPos(const OSG::Matrix camPos)
{
   OSG::beginEditCP(mTransform, OSG::Transform::MatrixFieldMask);
      mTransform->setMatrix(camPos);
   OSG::endEditCP(mTransform, OSG::Transform::MatrixFieldMask);
}

OSG::NodePtr FboVideoCamera::getDebugPlane() const
{
   OSG::SimpleMaterialPtr fbo_mat = OSG::SimpleMaterial::create();
   OSG::beginEditCP(fbo_mat);
      fbo_mat->addChunk(mFboTexture);
      fbo_mat->setSpecular(OSG::Color3f(0.7f, 0.7f, 0.7f));
      fbo_mat->setDiffuse(OSG::Color3f(0.22f, 0.2f, 0.2f));
   OSG::endEditCP(fbo_mat);

   // Create the test plane to put the texture on.
   OSG::GeometryPtr plane_geom = OSG::makePlaneGeo(10, 10, 2, 2);
   OSG::beginEditCP(plane_geom);
      plane_geom->setMaterial(fbo_mat);
   OSG::endEditCP(plane_geom);

   OSG::NodePtr plane_root = OSG::Node::create();
   OSG::beginEditCP(plane_root);
      plane_root->setCore(plane_geom);
   OSG::endEditCP(plane_root);
   return plane_root;
}

void FboVideoCamera::generateFrame()
{
   // The size of the internal frame.
   OSG::Real32 frame_height = 2.0 * (OSG::osgtan(mFboCam->getFov()/2.0) * mFrameDist);
   OSG::Real32 frame_width = frame_height * mFboCam->getAspect();

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
