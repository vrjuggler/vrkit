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

#if defined(WIN32) || defined(WIN64)
#  include <windows.h>
#endif

#if defined(__APPLE__)
#  include <OpenGL/glu.h>
#else
#  include <GL/glu.h>
#endif

#include <OpenSG/OSGSolidBackground.h>

#if OSG_MAJOR_VERSION >= 2
#  include <OpenSG/OSGRenderTraversalAction.h>
#else
#  include <OpenSG/OSGRenderAction.h>
#endif

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
#if OSG_MAJOR_VERSION >= 2
   , mFBO(OSG::NullFC)
   , mTexBuffer(OSG::NullFC)
#endif
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

#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor fvpe(mFboVP);
#endif
   mFboVP->setParent(window);
}

CameraPtr CameraFBO::init()
{
   CameraPtr cam_ptr = Camera::init();

   OSG::SolidBackgroundPtr bg = OSG::SolidBackground::create();
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor bge(bg, OSG::SolidBackground::ColorFieldMask);
#endif
   bg->setColor(OSG::Color3f(0, 0, 0));

   // create FBOViewport
   mFboVP = OSG::FBOViewport::create();
#if OSG_MAJOR_VERSION >= 2
   mTexBuffer = OSG::TextureBuffer::create();
   mFBO       = OSG::FrameBufferObject::create();
#endif

#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor fvpe(mFboVP);
#endif
   mFboVP->setBackground(bg);
   mFboVP->setCamera(mCamera);
#if OSG_MAJOR_VERSION < 2
   mFboVP->getTextures().push_back(mLeftTexture);
#else
   mFboVP->setFrameBufferObject(mFBO);
   mTexBuffer->setTexture(mLeftTexture);
#endif

   return cam_ptr;
}

void CameraFBO::setSceneRoot(OSG::NodePtr root)
{
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor fvpe(mFboVP, OSG::FBOViewport::RootFieldMask);
#endif
   mFboVP->setRoot(root);
}

void CameraFBO::setTravMask(const OSG::UInt32 value)
{
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor fvpe(mFboVP, OSG::FBOViewport::TravMaskFieldMask);
#endif
   mFboVP->setTravMask(value);
}

void CameraFBO::setSize(const OSG::UInt32 width, const OSG::UInt32 height)
{
   if( OSG::NullFC != mFboVP)
   {
      // Ensure that width and height are multiples of two. The values of
      // mWidth and mHeight set by this call are used later in render().
      Camera::setSize((width / 2) * 2, (height / 2) * 2);

      // Make sure this isn't the camera calling setsize from init
      // Resize the viewport.
#if OSG_MAJOR_VERSION < 2
      OSG::CPEditor fvpe(mFboVP);
#endif
      mFboVP->setSize(0, 0, width - 1, height - 1);
#if OSG_MAJOR_VERSION < 2
      mFboVP->setStorageWidth(width);
      mFboVP->setStorageHeight(height);
#else
      mFBO->setSize(width, height);
#endif
   }
}

void CameraFBO::render(render_action_t* ra)
{
#if OSG_MAJOR_VERSION < 2
   OSG::beginEditCP(mFboVP);
      mFboVP->getTextures()[0] = mCurrentTexture;
   OSG::endEditCP(mFboVP);
#else
   mTexBuffer->setTexture(mCurrentTexture);
#endif

#if OSG_MAJOR_VERSION >= 2
   OSG::DrawEnv draw_env;
   draw_env.setAction(ra);
   draw_env.setViewport(OSG::getCPtr(mFboVP));
   draw_env.setWindow(ra->getWindow());
   mTexBuffer->bind(&draw_env);
   mFBO->activate(&draw_env);
#endif

#if OSG_MAJOR_VERSION < 2
   // Do the actual rendering.
   glClear(GL_DEPTH_BUFFER_BIT);
   glPushAttrib(GL_ALL_ATTRIB_BITS);
      glPushMatrix();
         mFboVP->render(ra);
      glPopMatrix();
   glPopAttrib();

   mFboVP->bind(ra->getWindow());

   // If we are using an FBO, then we should change to the FBO buffer.
   glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
   checkGLError("before glReadPixels");
#endif

#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor cei(mCurrentImage);
#endif
   // Read the buffer into an OpenSG image.
   void* buffer =
#if OSG_MAJOR_VERSION < 2
      mCurrentImage->getData();
#else
      mCurrentImage->editData();
#endif
   glReadPixels(mFboVP->getPixelLeft(), mFboVP->getPixelBottom(),
                mWidth, mHeight, mCurrentImage->getPixelFormat(),
                GL_UNSIGNED_BYTE, buffer);
   checkGLError("after glReadPixels");

   // XXX: We don't really need to change the read buffer target since we
   //      are not reading from the pixel buffer anywhere else.
   // Double buffered.
   //glReadBuffer(GL_BACK);

#if OSG_MAJOR_VERSION < 2
   mFboVP->stop(ra->getWindow());
#else
   mFBO->deactivate(&draw_env);
#endif
}

}

}
