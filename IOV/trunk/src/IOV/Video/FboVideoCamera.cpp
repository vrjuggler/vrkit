// Copyright (C) Infiscape Corporation 2005-2007

#include <IOV/Video/FboVideoCamera.h>

#include <OpenSG/OSGFrustumVolume.h>
#include <OpenSG/OSGGeometry.h>
#include <OpenSG/OSGImage.h>
#include <OpenSG/OSGSimpleGeometry.h>
#include <OpenSG/OSGSimpleMaterial.h>
#include <OpenSG/OSGSolidBackground.h>
#include <OpenSG/OSGGeoPropPtrs.h>

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

FboVideoCameraPtr FboVideoCamera::init()
{
   // Create FBO camera xform.
   mFboCamTrans = OSG::Transform::create();
   mFboCamBeacon = OSG::Node::create();
   OSG::beginEditCP(mFboCamBeacon);
      mFboCamBeacon->setCore(mFboCamTrans);
   OSG::endEditCP(mFboCamBeacon);

   mFrameTrans = OSG::Transform::create();

   // Where should we attach the FBO camera beacon.
   //OSG::beginEditCP(fboRoot);
      //fboRoot->addChild(mFboCamBeacon);
   //OSG::endEditCP(fboRoot);

   // Create the FBO texture.
   mFboTexture = OSG::TextureChunk::create();
   OSG::beginEditCP(mFboTexture);
      mFboTexture->setEnvMode(GL_MODULATE);
   OSG::endEditCP(mFboTexture);

   setupFBO();

   return shared_from_this();
}

void FboVideoCamera::setupFBO()
{
   // setup FBO
   OSG::ImagePtr img = OSG::Image::create();
   OSG::beginEditCP(img);
      img->set(OSG::Image::OSG_RGBA_PF, mFboWidth, mFboHeight);
   OSG::endEditCP(img);
   OSG::beginEditCP(mFboTexture);
      mFboTexture->setMinFilter(GL_LINEAR);
      mFboTexture->setMagFilter(GL_LINEAR);
      mFboTexture->setTarget(GL_TEXTURE_2D);
      mFboTexture->setInternalFormat(GL_RGBA8);
      mFboTexture->setImage(img);
   OSG::endEditCP(mFboTexture);
   
   OSG::SolidBackgroundPtr bg = OSG::SolidBackground::create();
   OSG::beginEditCP(bg);
      bg->setColor(OSG::Color3f(0, 0, 0));
   OSG::endEditCP(bg);

   // setup camera
   mFboCam = OSG::PerspectiveCamera::create();
   OSG::beginEditCP(mFboCam);
      // we already compensated aspect ratio with the texture/fbo sizes
      mFboCam->setAspect(1.0);
      mFboCam->setFov(OSG::osgdegree2rad(60.0));
      mFboCam->setNear(0.01);
      mFboCam->setFar(10000);
      mFboCam->setBeacon(mFboCamBeacon);
   OSG::endEditCP(mFboCam);

    // create FBOViewport
   mFboVP = OSG::FBOViewport::create();
   OSG::beginEditCP(mFboVP);
      mFboVP->setSize(0, 0, mFboWidth - 1, mFboHeight - 1);
      mFboVP->setStorageWidth(mFboWidth);
      mFboVP->setStorageHeight(mFboHeight);
      mFboVP->setBackground(bg);
      mFboVP->setCamera(mFboCam);
      //mFboVP->setParent(gwin);
      // attach texture as render target
      mFboVP->getTextures().push_back(mFboTexture);
   OSG::endEditCP(mFboVP);
}

void FboVideoCamera::setCameraPos(OSG::Matrix camPos)
{
   //OSG::FrustumVolume vol;
   //mFboCam->getFrustum(vol, *mFboVP);
   //vol.dump();

   // set the fbo camera to show the whole model
   OSG::beginEditCP(mFboCamTrans, OSG::Transform::MatrixFieldMask);
   OSG::beginEditCP(mFrameTrans, OSG::Transform::MatrixFieldMask);
      mFboCamTrans->setMatrix(camPos);
      mFrameTrans->setMatrix(camPos);
   OSG::endEditCP(mFboCamTrans, OSG::Transform::MatrixFieldMask);
   OSG::endEditCP(mFrameTrans, OSG::Transform::MatrixFieldMask);
}

OSG::NodePtr FboVideoCamera::getDebugPlane()
{
   OSG::SimpleMaterialPtr fboMaterial = OSG::SimpleMaterial::create();
   OSG::beginEditCP(fboMaterial);
      fboMaterial->addChunk(mFboTexture);
      fboMaterial->setSpecular(OSG::Color3f(0.7f, 0.7f, 0.7f));
      fboMaterial->setDiffuse(OSG::Color3f(0.22f, 0.2f, 0.2f));
   OSG::endEditCP(fboMaterial);

   // Create the test plane to put the texture on.
   OSG::GeometryPtr plane_geom = OSG::makePlaneGeo(10, 10, 10, 10);
   OSG::beginEditCP(plane_geom);
      plane_geom->setMaterial(fboMaterial);
   OSG::endEditCP(plane_geom);

   OSG::NodePtr plane_root = OSG::Node::create();
   OSG::beginEditCP(plane_root);
      plane_root->setCore(plane_geom);
   OSG::endEditCP(plane_root);
   return plane_root;
}

OSG::NodePtr FboVideoCamera::getFrame()
{
   float far = 100.0;
   float frame_size = OSG::osgatan(mFboCam->getFov()/2.0) * far * 2.25;
   float width = 2.0;

   OSG::GeometryNodePtr top = OSG::GeometryNodePtr::create();
   OSG::GeometryNodePtr bottom = OSG::GeometryNodePtr::create();
   OSG::GeometryNodePtr left = OSG::GeometryNodePtr::create();
   OSG::GeometryNodePtr right = OSG::GeometryNodePtr::create();

   top = OSG::makeBox(frame_size+width+width, width, width, 2, 2, 2);
   bottom = OSG::makeBox(frame_size+width+width, width, width, 2, 2, 2);
   left = OSG::makeBox(width, frame_size, width, 2, 2, 2);
   right = OSG::makeBox(width, frame_size, width, 2, 2, 2);

   OSG::TransformNodePtr top_xform = OSG::TransformNodePtr::create();
   OSG::TransformNodePtr bottom_xform = OSG::TransformNodePtr::create();
   OSG::TransformNodePtr left_xform = OSG::TransformNodePtr::create();
   OSG::TransformNodePtr right_xform = OSG::TransformNodePtr::create();

   // Create material for the frame.
   OSG::SimpleMaterialPtr mat = OSG::SimpleMaterial::create();
   OSG::beginEditCP(mat);
      mat->setLit(false);
      mat->setDiffuse(OSG::Color3f(1.0f, 0.0f, 0.0f));
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

   // Create the xforms for each box.
   OSG::Matrix topm, bottomm, leftm, rightm;
   topm.setTranslate(0.0, (frame_size/2.0) + (width/2.0), -far);
   bottomm.setTranslate(0.0, (-frame_size/2.0) - (width/2.0), -far);
   leftm.setTranslate((-frame_size/2.0) - (width/2.0), 0.0, -far);
   rightm.setTranslate((frame_size/2.0) + (width/2.0), 0.0, -far);

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

   OSG::NodePtr frame_root = OSG::Node::create();
   OSG::beginEditCP(frame_root);
      frame_root->setCore(mFrameTrans);
      frame_root->addChild(top_xform.node());
      frame_root->addChild(bottom_xform.node());
      frame_root->addChild(left_xform.node());
      frame_root->addChild(right_xform.node());
   OSG::endEditCP(frame_root);

   return frame_root;
}

}
