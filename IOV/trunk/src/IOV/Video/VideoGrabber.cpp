// Copyright (C) Infiscape Corporation 2005-2007

#include <OpenSG/OSGFBOViewport.h>

#include <IOV/Video/VideoGrabber.h>
#include <IOV/Video/FfmpegEncoder.h>

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

VideoGrabberPtr VideoGrabber::create()
{
   return VideoGrabberPtr(new VideoGrabber);
}

VideoGrabber::~VideoGrabber()
{
#ifdef IOV_WITH_FFMPEG
   if (NULL != mEncoder.get())
   {
      mEncoder->close();
      mEncoder = FfmpegEncoderPtr();
   }
#endif
   mImage = OSG::NullFC;
   mViewport = OSG::NullFC;
}

VideoGrabberPtr VideoGrabber::init(OSG::ViewportPtr viewport)
{
   mViewport = viewport;

   if (mViewport->getType() == OSG::FBOViewport::getClassType())
   {
      mUseFbo = true;
   }

   return shared_from_this();
}

void VideoGrabber::record(const std::string& filename, const OSG::UInt32 fps)
{
   OSG::UInt32 width = ( mViewport->getPixelWidth() / 2 ) * 2;
   OSG::UInt32 height = ( mViewport->getPixelHeight() / 2 ) * 2;

#ifdef IOV_WITH_FFMPEG
   if (NULL != mEncoder.get())
   {
      mEncoder->close();
   }
   mEncoder = FfmpegEncoder::create()->init(filename, width, height, fps);
#endif

   mRecording = true;
}

void VideoGrabber::pause()
{
#ifdef IOV_WITH_FFMPEG
   OSG_ASSERT(NULL != mEncoder.get() && "Can't pause if we aren't recording.");
#endif
   mRecording = false;
}

void VideoGrabber::resume()
{
#ifdef IOV_WITH_FFMPEG
   OSG_ASSERT(NULL != mEncoder.get() && "Can't resume if we aren't recording.");
   if (NULL != mEncoder.get())
   {
      mRecording = true;
   }
#endif
}

void VideoGrabber::stop()
{
#ifdef IOV_WITH_FFMPEG
   OSG_ASSERT(NULL != mEncoder.get() && "Can't stop if we aren't recording.");
   if (NULL != mEncoder.get())
   {
      mEncoder->close();
      mEncoder = FfmpegEncoderPtr();
   }
#endif
   mRecording = false;
}

void VideoGrabber::draw()
{
   if (!mRecording)
   {
      return;
   }

   if (OSG::NullFC == mImage)
   {
      mImage = OSG::Image::create();
      OSG::beginEditCP(mImage);
         mImage->set(OSG::Image::OSG_RGB_PF, 1);
      OSG::endEditCP(mImage);
   }

   OSG::UInt32 w = ( mViewport->getPixelWidth() / 2 ) * 2;
   OSG::UInt32 h = ( mViewport->getPixelHeight() / 2 ) * 2;


   if(mImage->getWidth() <= 1 && mImage->getHeight() <= 1)
   {
      mImage->set(mImage->getPixelFormat(), w, h);
   }

#ifdef IOV_WITH_FFMPEG
   mEncoder->setRgb(mImage->getData());

   bool storeChanged = false;
   if(mEncoder->width() != mViewport->getPixelWidth() )
   {
      glPixelStorei(GL_PACK_ROW_LENGTH, mEncoder->width());
      storeChanged = true;
   }

   if (mUseFbo)
   {
      checkGLError("before glReadBuffer 1");
      glReadBuffer(GL_COLOR_ATTACHMENT0_EXT); // FBO version
      checkGLError("before glReadPixels 1");
   }

   // Read the buffer into an OpenSG image.
   glReadPixels(mViewport->getPixelLeft(), mViewport->getPixelBottom(),
                mEncoder->width(), mEncoder->height(), mImage->getPixelFormat(),
                GL_UNSIGNED_BYTE, mImage->getData());
   // Double buffered.
   //glReadBuffer(GL_BACK);

   if(storeChanged)
   {
      glPixelStorei(GL_PACK_ROW_LENGTH, 0);
   }

   mEncoder->writeFrame();
#endif
}

}
