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

#include <OpenSG/OSGSolidBackground.h>

#include <vrkit/video/CameraFBO.h>

#ifndef GL_COLOR_ATTACHMENT0_EXT
#  define GL_COLOR_ATTACHMENT0_EXT 0x8CE0
#endif


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

namespace vrkit
{

namespace video
{

CameraFBO::CameraFBO()
   : Camera()
   , mFboVP(OSG::NullFC)
{
   /* Do nothing. */ ;
}

CameraFBOPtr CameraFBO::create()
{
   return CameraFBOPtr(new CameraFBO);
}

CameraFBO::~CameraFBO()
{
   /* Do nothing. */ ;
}

void CameraFBO::setWindow(OSG::WindowPtr window)
{
   Camera::setWindow(window);

   OSG::beginEditCP(mFboVP);
   mFboVP->setParent(window);
   OSG::endEditCP(mFboVP);
}

CameraPtr CameraFBO::init()
{
   CameraPtr cam_ptr = Camera::init();

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

   return cam_ptr;
}

void CameraFBO::setSceneRoot(OSG::NodePtr root)
{
   OSG::beginEditCP(mFboVP);
      mFboVP->setRoot(root);
   OSG::endEditCP(mFboVP);
}

void CameraFBO::setTravMask(const OSG::UInt32 value)
{
   OSG::beginEditCP(mFboVP);
      mFboVP->setTravMask(value);
   OSG::endEditCP(mFboVP);
}

void CameraFBO::setSize(const OSG::UInt32 width, const OSG::UInt32 height)
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
   }
}

void CameraFBO::render(OSG::RenderAction* ra)
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

   // If we are using an FBO, then we should change to the FBO buffer.
   glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
   checkGLError("before glReadPixels");

   // Read the buffer into an OpenSG image.
   glReadPixels(mFboVP->getPixelLeft(), mFboVP->getPixelBottom(),
                source_width, source_height, mCurrentImage->getPixelFormat(),
                GL_UNSIGNED_BYTE, mCurrentImage->getData());
   checkGLError("after glReadPixels");

   // XXX: We don't really need to change the read buffer target since we
   //      are not reading from the pixel buffer anywhere else.
   // Double buffered.
   //glReadBuffer(GL_BACK);

   mFboVP->stop(ra->getWindow());
}

}

}
