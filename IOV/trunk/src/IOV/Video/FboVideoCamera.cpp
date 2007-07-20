// Copyright (C) Infiscape Corporation 2005-2007

#include <IOV/Video/FboVideoCamera.h>

#include <OpenSG/OSGGeometry.h>
#include <OpenSG/OSGImage.h>
#include <OpenSG/OSGPerspectiveCamera.h>
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

FboVideoCameraPtr FboVideoCamera::init()
{
   // Create FBO camera xform.
   mFboCamTrans = OSG::Transform::create();
   mFboCamBeacon = OSG::Node::create();
   OSG::beginEditCP(mFboCamBeacon);
      mFboCamBeacon->setCore(mFboCamTrans);
   OSG::endEditCP(mFboCamBeacon);

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
   OSG::PerspectiveCameraPtr fboCam = OSG::PerspectiveCamera::create();
   OSG::beginEditCP(fboCam);
      // we already compensated aspect ratio with the texture/fbo sizes
      fboCam->setAspect(1.0);
      fboCam->setFov(OSG::osgdegree2rad(60));
      fboCam->setNear(0.01);
      fboCam->setFar(100);
      fboCam->setBeacon(mFboCamBeacon);
   OSG::endEditCP(fboCam);

    // create FBOViewport
   mFboVP = OSG::FBOViewport::create();
   OSG::beginEditCP(mFboVP);
      mFboVP->setSize(0, 0, mFboWidth - 1, mFboHeight - 1);
      mFboVP->setStorageWidth(mFboWidth);
      mFboVP->setStorageHeight(mFboHeight);
      mFboVP->setBackground(bg);
      mFboVP->setCamera(fboCam);
      //mFboVP->setParent(gwin);
      // attach texture as render target
      mFboVP->getTextures().push_back(mFboTexture);
   OSG::endEditCP(mFboVP);
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

}
