// Copyright (C) Infiscape Corporation 2005-2007

#include <sstream>
#include <OpenSG/OSGFBOViewport.h>
#include <OpenSG/OSGGLExt.h>

#include <IOV/Util/Exceptions.h>
#include <IOV/Video/VideoGrabber.h>

#ifndef GL_COLOR_ATTACHMENT0_EXT
#   define GL_COLOR_ATTACHMENT0_EXT 0x8CE0
#endif

#ifdef IOV_WITH_FFMPEG
#   include <IOV/Video/FfmpegEncoder.h>
#elif defined(IOV_WITH_VFW)
#   include <IOV/Video/VfwEncoder.h>
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

namespace inf
{

VideoGrabberPtr VideoGrabber::create()
{
   return VideoGrabberPtr(new VideoGrabber);
}

VideoGrabber::~VideoGrabber()
{
   if (NULL != mEncoder.get())
   {
      mEncoder->close();
      mEncoder = EncoderPtr();
   }
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

#ifdef IOV_WITH_FFMPEG
   const std::string encoder_name = VfwEncoder::getName();

   // Register the creator.
   mCreatorMap[encoder_name] = &VfwEncoder::create;

   // Register each codec type.
   VfwEncoder::codec_list_t cl = VfwEncoder::getCodecs();
   for (VfwEncoder::codec_list_t::const_iterator itr = cl.begin(); 
        itr != cl.end(); ++itr)
   {
      mCodecMap[*itr].push_back(encoder_name);
   }
#endif

#ifdef IOV_WITH_VFW
   const std::string encoder_name = VfwEncoder::getName();

   // Register the creator.
   mCreatorMap[encoder_name] = &VfwEncoder::create;

   // Register each codec type.
   VfwEncoder::codec_list_t cl = VfwEncoder::getCodecs();
   for (VfwEncoder::codec_list_t::const_iterator itr = cl.begin(); 
        itr != cl.end(); ++itr)
   {
      mCodecMap[*itr].push_back(encoder_name);
   }
#endif

   return shared_from_this();
}

void VideoGrabber::record(const std::string& filename, const std::string& codec,
                          const OSG::UInt32 framesPerSecond)
{
   OSG::UInt32 width = ( mViewport->getPixelWidth() / 2 ) * 2;
   OSG::UInt32 height = ( mViewport->getPixelHeight() / 2 ) * 2;

   codec_map_t::const_iterator found = mCodecMap.find(codec);
   if (mCodecMap.end() == found)
   {
      std::stringstream ss;
      ss << "Can't find encoder for codec: " << codec;
      throw inf::Exception(ss.str(), IOV_LOCATION);
   }

   if (NULL != mEncoder.get())
   {
      mEncoder->close();
   }

   // Get the first encoder.
   std::string encoder_name = (*found).second[0];
   vprASSERT(mCreatorMap.count(encoder_name) > 0 && "Must have the encoder.");
   // Create new encoder.
   encoder_create_t creator = mCreatorMap[encoder_name];
   mEncoder = creator()->init(filename, codec, width, height, framesPerSecond);

   mRecording = true;
}

void VideoGrabber::pause()
{
   OSG_ASSERT(NULL != mEncoder.get() && "Can't pause if we aren't recording.");
   mRecording = false;
}

void VideoGrabber::resume()
{
   OSG_ASSERT(NULL != mEncoder.get() && "Can't resume if we aren't recording.");
   if (NULL != mEncoder.get())
   {
      mRecording = true;
   }
}

void VideoGrabber::stop()
{
   OSG_ASSERT(NULL != mEncoder.get() && "Can't stop if we aren't recording.");
   if (NULL != mEncoder.get())
   {
      mEncoder->close();
      mEncoder = EncoderPtr();
   }
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
// Video for Windows wants bytes in BRG order. Ask the GL driver for them in that order.
#if defined(IOV_WITH_VFW)
         mImage->set(OSG::Image::OSG_BGR_PF, 1);
#else
         mImage->set(OSG::Image::OSG_RGB_PF, 1);
#endif
      OSG::endEditCP(mImage);
   }

   OSG::UInt32 w = ( mViewport->getPixelWidth() / 2 ) * 2;
   OSG::UInt32 h = ( mViewport->getPixelHeight() / 2 ) * 2;


   if(mImage->getWidth() <= 1 && mImage->getHeight() <= 1)
   {
      mImage->set(mImage->getPixelFormat(), w, h);
   }

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

   mEncoder->writeFrame(mEncoder->width(), mEncoder->height(), mImage->getData());
}

}
