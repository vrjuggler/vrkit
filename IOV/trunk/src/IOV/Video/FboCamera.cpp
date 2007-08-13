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

namespace
{
   bool checkGLError(const char* where)
   {
      GLenum errCode = 0;
      if ((errCode = glGetError()) != GL_NO_ERROR)
      {
         const GLubyte *errString = gluErrorString(errCode);
         FWARNING(("%s OpenGL Error: %s!\n", where, errString));
      }

      return errCode == GL_NO_ERROR;
   }
}

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

void FboCamera::setWindow(OSG::WindowPtr window)
{
   Camera::setWindow(window);

   OSG::beginEditCP(mFboVP);
   mFboVP->setParent(window);
   OSG::endEditCP(mFboVP);
}

CameraPtr FboCamera::init()
{
   CameraPtr cam_ptr = inf::Camera::init();

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
   inf::FboCamera::setSize(mWidth, mHeight);

   return cam_ptr;
}

void FboCamera::setSceneRoot(OSG::NodePtr root)
{
   OSG::beginEditCP(mFboVP);
      mFboVP->setRoot(root);
   OSG::endEditCP(mFboVP);
}

void FboCamera::setSize(const OSG::UInt32 width, const OSG::UInt32 height)
{ 
   if( OSG::NullFC != mFboVP)
   {
      Camera::setSize(width, height);
      
      // Make sure this isn't the camera calling setsize from init
      // Resize the viewport.
      OSG::beginEditCP(mFboVP);
	 mFboVP->setSize(0, 0, width - 1, height - 1);
	 mFboVP->setStorageWidth(width);
	 mFboVP->setStorageHeight(height);
      OSG::endEditCP(mFboVP);

      generateDebugFrame();
   }
}

void FboCamera::render(OSG::RenderAction* ra)
{
   OSG::beginEditCP(mFboVP);
      mFboVP->getTextures()[0] = mCurrentTexture;
   OSG::endEditCP(mFboVP);

   // Do the actual rendering.
   glClear(GL_DEPTH_BUFFER_BIT);
   glPushAttrib(GL_ALL_ATTRIB_BITS);
      glPushMatrix();
	 mFboVP->render(ra);
      glPopMatrix();
   glPopAttrib();

   mFboVP->bind(ra->getWindow());

   OSG::UInt32 source_width = ( mWidth / 2 ) * 2;
   OSG::UInt32 source_height = ( mHeight / 2 ) * 2;

   OSG::UInt32 image_width = source_width;
   OSG::UInt32 image_height = source_height;

   // Tell the OpenGL driver the row length of the target image. This
   // will result in the driver getting the array indices with the
   // following formula. index = SP + (IR * RL) + IC.
   // Where:
   //   IR = Input pixel row.
   //   IC = Input pixel column.
   //   SP = GL_PACK_SKIP_PIXELS
   //   RL = GL_PACK_ROW_LENGTH
   glPixelStorei(GL_PACK_ROW_LENGTH, image_width);

   // If we are using an FBO, then we should change to the FBO buffer.
   glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
   checkGLError("before glReadPixels");

   // Read the buffer into an OpenSG image.
   glReadPixels(mFboVP->getPixelLeft(), mFboVP->getPixelBottom(),
                source_width, source_height, mCurrentImage->getPixelFormat(),
                GL_UNSIGNED_BYTE, mCurrentImage->getData());

   // XXX: We don't really need to change the read buffer target since we
   //      are not reading from the pixel buffer anywhere else.
   // Double buffered.
   //glReadBuffer(GL_BACK);

   // Restore the pixel storage settings to the default.
   glPixelStorei(GL_PACK_ROW_LENGTH, 0);
   //glPixelStorei(GL_PACK_SKIP_PIXELS, 0);

   mFboVP->stop(ra->getWindow());
}

}
