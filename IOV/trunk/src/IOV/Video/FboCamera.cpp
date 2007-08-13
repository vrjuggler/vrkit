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
#include <IOV/Video/FboCamera.h>

namespace inf
{

FboCamera::FboCamera()
   : inf::Camera()
   , mFboVP(OSG::NullFC)
{;}

FboCameraPtr FboCamera::create()
{
   return FboCameraPtr(new FboCamera);
}

FboCamera::~FboCamera()
{
}

void FboCamera::contextInit(OSG::WindowPtr window)
{
   mFboVP->setParent(window);
}

CameraPtr FboCamera::init()
{
   inf::Camera::init();

   OSG::SolidBackgroundPtr bg = OSG::SolidBackground::create();
   OSG::beginEditCP(bg);
      bg->setColor(OSG::Color3f(0, 0, 0));
   OSG::endEditCP(bg);

   // create FBOViewport
   mFboVP = OSG::FBOViewport::create();
   OSG::beginEditCP(mFboVP);
      mFboVP->setBackground(bg);
      mFboVP->setCamera(mCamera);
      mFboVP->getTextures().push_back(mLeftTexture);
   OSG::endEditCP(mFboVP);

   // Set the correct size of FBO.
   // This also generates the frame geometry around the captured scene.
   setSize(mWidth, mHeight);

   return shared_from_this();
}

void FboCamera::setSceneRoot(OSG::NodePtr root)
{
   OSG::beginEditCP(mFboVP);
      mFboVP->setRoot(root);
   OSG::endEditCP(mFboVP);
}

void FboCamera::setSize(const OSG::UInt32 width, const OSG::UInt32 height)
{ 
   Camera::setSize(width, height);

   // Resize the viewport.
   OSG::beginEditCP(mFboVP);
      mFboVP->setSize(0, 0, width - 1, height - 1);
      mFboVP->setStorageWidth(width);
      mFboVP->setStorageHeight(height);
   OSG::endEditCP(mFboVP);


   generateDebugFrame();
}

void FboCamera::render(OSG::RenderAction* ra)
{
   // Do the actual rendering.
   glClear(GL_DEPTH_BUFFER_BIT);
   glPushAttrib(GL_ALL_ATTRIB_BITS);
      glPushMatrix();
	 mFboVP->render(ra);
      glPopMatrix();
   glPopAttrib();

   // XXX: Only capturing right eye right now.
   mFboVP->bind(ra->getWindow());
   //XXX stuff image!! mVideoGrabber->grabFrame(true);
   mFboVP->stop(ra->getWindow());
}

}
