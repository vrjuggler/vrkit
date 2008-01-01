// vrkit is (C) Copyright 2005-2008
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

#include <algorithm>
#include <boost/bind.hpp>

#include <OpenSG/OSGGeometry.h>
#if OSG_MAJOR_VERSION < 2
#  include <OpenSG/OSGGeoPropPtrs.h>
#endif
#include <OpenSG/OSGImage.h>
#include <OpenSG/OSGMergeGraphOp.h>
#include <OpenSG/OSGGraphOpSeq.h>
#include <OpenSG/OSGSimpleGeometry.h>
#include <OpenSG/OSGSimpleMaterial.h>

#include <vrkit/Status.h>
#include <vrkit/Exception.h>

#ifdef VRKIT_WITH_FFMPEG
#  include <vrkit/video/EncoderFFmpeg.h>
#endif

#if defined(VRKIT_WITH_VFW)
#  include <vrkit/video/EncoderVFW.h>
#endif

#if defined(VRKIT_WITH_DIRECT_SHOW)
#  include <vrkit/video/EncoderDirectShow.h>
#endif

#include <vrkit/video/CameraFBO.h>
#include <vrkit/video/Recorder.h>

#define REGISTER_ENCODER(ENCODER)                                       \
   EncoderPtr encoder_ ## ENCODER = ENCODER::create()->init();          \
   mEncoderMap[ENCODER::getName()] = encoder_ ## ENCODER;               \
                                                                        \
   Encoder::container_format_list_t enc_fmt_list ## ENCODER =           \
      encoder_ ## ENCODER->getSupportedContainersAndCodecs();           \
   /* Register each container format list. */                           \
   mVideoEncoderFormatList.insert(mVideoEncoderFormatList.end(),        \
                                  enc_fmt_list ## ENCODER.begin(),      \
                                  enc_fmt_list ## ENCODER.end());

#ifndef GL_COLOR_ATTACHMENT0_EXT
#  define GL_COLOR_ATTACHMENT0_EXT 0x8CE0
#endif


namespace vrkit
{

namespace video
{

Recorder::Recorder()
   : mCamera()
   , mStereoImageStorage(OSG::NullFC)
   , mTransform(OSG::NullFC)
   , mFrameRoot(OSG::NullFC)
   , mEyeOffset(0.5)
   , mBorderSize(2.0)
   , mFrameDist(50.0)
   , mDrawScale(1.0)
   , mRecording(false)
   , mPaused(false)
   , mStereo(false)
   , mFilename("vrkit_movie.avi")
   , mFps(30)
   , mWidth(512)
   , mHeight(512)
{
   ;
}

RecorderPtr Recorder::create()
{
   return RecorderPtr(new Recorder);
}

RecorderPtr Recorder::init()
{
   // Create a transform to contain the location and orientation of the camera.
   mTransform = OSG::Transform::create();

   OSG::NodePtr beacon = OSG::Node::create();
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor be(beacon, OSG::Node::CoreFieldMask);
#endif
   beacon->setCore(mTransform);

   // Create the frame root.
   mFrameRoot = OSG::Node::create();
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor fre(mFrameRoot, OSG::Node::CoreFieldMask);
#endif
   mFrameRoot->setCore(mTransform);

   mStereoImageStorage = OSG::Image::create();

   // TODO: Do not hard code the use of CameraFBO.
   mCamera = CameraFBO::create()->init();

   assert(mCamera.get() != NULL);

#ifdef VRKIT_WITH_FFMPEG
   try
   {
      REGISTER_ENCODER(EncoderFFmpeg)
   }
   catch (Exception& ex)
   {
      VRKIT_STATUS << "Failed to register FFmpeg encoder:\n" << ex.what()
                   << std::endl;
   }
#endif

#ifdef VRKIT_WITH_VFW
   try
   {
      REGISTER_ENCODER(EncoderVFW)
   }
   catch (Exception& ex)
   {
      VRKIT_STATUS << "Failed to register VFW encoder:\n" << ex.what()
                   << std::endl;
   }
#endif

#ifdef VRKIT_WITH_DIRECT_SHOW
   try
   {
      REGISTER_ENCODER(EncoderDirectShow)
   }
   catch (Exception& ex)
   {
      VRKIT_STATUS << "Failed to register DirectShow encoder:\n" << ex.what()
                   << std::endl;
   }
#endif

   return shared_from_this();
}

Recorder::~Recorder()
{
   if ( NULL != mEncoder.get() )
   {
      mEncoder->stopEncoding();
      mEncoder = EncoderPtr();
   }
}

void Recorder::setFormat(const VideoEncoderFormat& format)
{
   mVideoEncoderParams = format;
}

void Recorder::setFilename(const std::string& filename)
{
   mFilename = filename;
}

void Recorder::setFov(const OSG::Real32 fov)
{
   mCamera->setFov(
#if OSG_MAJOR_VERSION < 2
      OSG::osgdegree2rad(fov)
#else
      OSG::osgDegree2Rad(fov)
#endif
   );
}

void Recorder::setAspect(const OSG::Real32 aspect)
{
   mCamera->setAspect(aspect);
}

void Recorder::setFramesPerSecond(const OSG::UInt32 framesPerSecond)
{
   mFps = framesPerSecond;
}

void Recorder::setFrameSize(const OSG::UInt32 width, const OSG::UInt32 height)
{
   mCamera->setSize(width, height);

   mWidth  = width;
   mHeight = height;

   generateDebugFrame(); //XXX
}

void Recorder::setInterocularDistance(const OSG::Real32 interocular)
{
   mEyeOffset = interocular / 2.0f;
}

void Recorder::setNearFar(const OSG::Real32 nearVal, const OSG::Real32 farVal)
{
   mCamera->setNearFar(nearVal, farVal);
}

void Recorder::setSceneRoot(OSG::NodePtr root)
{
   mCamera->setSceneRoot(root);
}

void Recorder::setDrawScaleFactor(const float scale)
{
   mDrawScale = scale;
   generateDebugFrame();
}

void Recorder::setStereo(const bool stereo)
{
   mStereo = stereo;
}

void Recorder::setTravMask(const OSG::UInt32 value)
{
   mCamera->setTravMask(value);
}

void Recorder::startRecording()
{
   if ( ! isRecording() )
   {
      // Ensure that recording actually started.
      if ( startEncoder() )
      {
         const OSG::Image::PixelFormat pix_format(mEncoder->getPixelFormat());

         mCamera->setPixelFormat(pix_format);

         if ( inStereo() )
         {
#if OSG_MAJOR_VERSION < 2
            OSG::CPEditor sise(mStereoImageStorage);
#endif
            mStereoImageStorage->set(pix_format, mCamera->getWidth() * 2,
                                     mCamera->getHeight());
         }
      }
   }
}

void Recorder::pause()
{
   if ( isRecording() && ! isPaused() )
   {
      mPaused = true;
      mRecordingPaused();
   }
}

void Recorder::resume()
{
   if ( isPaused() )
   {
      mPaused = false;
      mRecordingResumed();
   }
}

void Recorder::endRecording()
{
   if ( isRecording() )
   {
      mEncoder->stopEncoding();
      mEncoder = EncoderPtr();
      mRecordingStopped();
   }

   mRecording = false;
   mPaused    = false;
}

void Recorder::contextInit(OSG::WindowPtr window)
{
   mCamera->setWindow(window);
}

void Recorder::render(render_action_t* ra, const OSG::Matrix& camPos)
{
   if ( ! isRecording() || isPaused() )
   {
      return;
   }

   // If we are rendering stereo, then offset the camera position.
   if ( inStereo() )
   {
      OSG::Matrix offset;
      OSG::Matrix camera_pos;

      camera_pos = camPos;
      offset.setTranslate(-mEyeOffset, 0.0f, 0.0f);
      camera_pos.multLeft(offset);
      setCameraPos(camera_pos);

      mCamera->renderLeftEye(ra);

      // Set the correct camera position.
      camera_pos = camPos;
      offset.setTranslate(mEyeOffset, 0.0f, 0.0f);
      camera_pos.multLeft(offset);
      setCameraPos(camera_pos);

      mCamera->renderRightEye(ra);

      const OSG::UInt32 width = mCamera->getWidth();
      const OSG::UInt32 height = mCamera->getHeight();

#if OSG_MAJOR_VERSION < 2
      OSG::CPEditor sise(mStereoImageStorage);
#endif
      mStereoImageStorage->setSubData(
         0, 0, 0, width, height, 1, mCamera->getLeftEyeImage()->getData()
      );
      mStereoImageStorage->setSubData(
         width, 0, 0, width, height, 1, mCamera->getRightEyeImage()->getData()
      );

      writeFrame(mStereoImageStorage);
   }
   else
   {
      setCameraPos(camPos);
      mCamera->renderLeftEye(ra);
      writeFrame(mCamera->getLeftEyeImage());
   }
}

OSG::NodePtr Recorder::getDebugPlane() const
{
   // Create material for left eye.
   OSG::SimpleMaterialPtr left_mat = OSG::SimpleMaterial::create();
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor lme(left_mat);
#endif
   left_mat->addChunk(mCamera->getLeftTexture());
#if OSG_MAJOR_VERSION >= 2
   left_mat->addChunk(mCamera->getLeftTextureEnv());
#endif
   left_mat->setSpecular(OSG::Color3f(0.7f, 0.7f, 0.7f));
   left_mat->setDiffuse(OSG::Color3f(0.22f, 0.2f, 0.2f));

   // Create material for right eye.
   OSG::SimpleMaterialPtr right_mat = OSG::SimpleMaterial::create();
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor rme(right_mat);
#endif
   right_mat->addChunk(mCamera->getRightTexture());
#if OSG_MAJOR_VERSION >= 2
   right_mat->addChunk(mCamera->getRightTextureEnv());
#endif
   right_mat->setSpecular(OSG::Color3f(0.7f, 0.7f, 0.7f));
   right_mat->setDiffuse(OSG::Color3f(0.22f, 0.2f, 0.2f));

   // Create geometry for left eye.
   OSG::GeometryPtr left_geom = OSG::makePlaneGeo(5, 5, 2, 2);
   OSG::NodePtr left_node = OSG::Node::create();
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor lge(left_geom, OSG::Geometry::MaterialFieldMask);
   OSG::CPEditor lne(left_node, OSG::Node::CoreFieldMask);
#endif
   left_geom->setMaterial(left_mat);
   left_node->setCore(left_geom);

   // Create geometry for right eye.
   OSG::GeometryPtr right_geom = OSG::makePlaneGeo(5, 5, 2, 2);
   OSG::NodePtr right_node = OSG::Node::create();
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor rge(right_geom, OSG::Geometry::MaterialFieldMask);
   OSG::CPEditor rne(right_node, OSG::Node::CoreFieldMask);
#endif
   right_geom->setMaterial(right_mat);
   right_node->setCore(right_geom);

   // Create the xforms for each eye's geometry.
   OSG::Matrix leftm, rightm;
   leftm.setTranslate(-2.5, 2.5, 0.0);
   rightm.setTranslate(2.5, 2.5, 0.0);
   leftm.setScale(mDrawScale);
   rightm.setScale(mDrawScale);

   // Create the transform nodes for each eye's geometry.
   OSG::TransformNodePtr left_xform = OSG::TransformNodePtr::create();
   OSG::TransformNodePtr right_xform = OSG::TransformNodePtr::create();
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor lxe(left_xform.node(), OSG::Node::ChildrenFieldMask);
   OSG::CPEditor lxce(left_xform.core(), OSG::Transform::MatrixFieldMask);
   OSG::CPEditor rxe(right_xform.node(), OSG::Node::ChildrenFieldMask);
   OSG::CPEditor rxce(right_xform.core(), OSG::Transform::MatrixFieldMask);
#endif
   left_xform->setMatrix(leftm);
   right_xform->setMatrix(rightm);
   left_xform.node()->addChild(left_node);
   right_xform.node()->addChild(right_node);

   OSG::NodePtr group_node = OSG::Node::create();
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor gne(
      group_node, OSG::Node::ChildrenFieldMask | OSG::Node::CoreFieldMask
   );
#endif
   group_node->setCore(OSG::Group::create());
   group_node->addChild(left_xform.node());
   group_node->addChild(right_xform.node());

   return group_node;
}

void Recorder::setDebugFrameBorderWidth(const float value)
{
   mBorderSize = value;
   generateDebugFrame();
}

void Recorder::setDebugFrameDistance(const float value)
{
   mFrameDist = value;
   generateDebugFrame();
}

OSG::NodePtr Recorder::getFrame() const
{
   return mFrameRoot;
}

bool Recorder::startEncoder()
{
#if 0
   codec_map_t::const_iterator found = mCodecMap.find(mCodec);
   if (mCodecMap.end() == found)
   {
      std::stringstream ss;
      ss << "Can't find encoder for codec: " << mCodec;
      throw Exception(ss.str(), VRKIT_LOCATION);
   }
#endif

   if ( NULL != mEncoder.get() )
   {
      mEncoder->stopEncoding();
      mEncoder = EncoderPtr();
   }

   bool started(false);

   if ( mEncoderMap.count(mVideoEncoderParams.mEncoderName) > 0 )
   {
      encoder_map_t::const_iterator vid_encoder =
         mEncoderMap.find(mVideoEncoderParams.mEncoderName);
      mEncoder = (*vid_encoder).second;

      Encoder::EncoderParameters encoder_params =
         {
            mVideoEncoderParams.mContainerFormat,
            mVideoEncoderParams.mCodec,
            mFilename,
            mStereo ? mWidth * 2 : mWidth,
            mHeight,
            mFps
         };

      mEncoder->setEncodingParameters(encoder_params);
      mEncoder->startEncoding();

      mRecording = true;
      mRecordingStarted();
      started = true;
   }

   return started;
}

void Recorder::setCameraPos(const OSG::Matrix& camPos)
{
   mCamera->setPosition(camPos);
}

void Recorder::writeFrame(OSG::ImagePtr img)
{
   if ( ! mRecording )
   {
      return;
   }

   try
   {
      mEncoder->writeFrame(img->getData());
   }
   catch (std::exception& ex)
   {
      std::cerr << "Encoder failed to write frame; stopping encoding\n"
                << ex.what() << std::endl;
      endRecording();
   }
}

// XXX: This has not been updated to behave correctly in stereo mode.
void Recorder::generateDebugFrame()
{
   // The size of the internal frame.
   const OSG::Real32 fov_tan(
#if OSG_MAJOR_VERSION < 2
      OSG::osgtan(mCamera->getFov() / 2.0)
#else
      OSG::osgTan(mCamera->getFov() / 2.0)
#endif
   );
   const OSG::Real32 frame_height(2.0 * fov_tan * mFrameDist);
   const OSG::Real32 frame_width(frame_height * mCamera->getAspect());

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
   {
#if OSG_MAJOR_VERSION < 2
      OSG::CPEditor me(mat);
#endif
      //mat->setLit(false);
      //mat->setDiffuse(OSG::Color3f(1.0f, 0.0f, 0.0f));
      //mat->setAmbient(OSG::Color3f(0.2f, 0.2f, 0.2f));
      mat->setDiffuse(OSG::Color3f(0.8f, 0.0f, 0.0f));
      //mat->setSpecular(OSG::Color3f(0.2f, 0.2f, 0.2f));
      //mat->setShininess(2);
   }

   // Set the material for all parts of the frame.
   {
#if OSG_MAJOR_VERSION < 2
      OSG::CPEditor te(top.core(), OSG::Geometry::MaterialFieldMask);
      OSG::CPEditor be(bottom.core(), OSG::Geometry::MaterialFieldMask);
      OSG::CPEditor le(left.core(), OSG::Geometry::MaterialFieldMask);
      OSG::CPEditor re(right.core(), OSG::Geometry::MaterialFieldMask);
#endif
      top->setMaterial(mat);
      bottom->setMaterial(mat);
      left->setMaterial(mat);
      right->setMaterial(mat);
   }

   // We need to pull the frame in half the border size to ensure that we don't
   // see it.
   OSG::Real32 zoffset = -mFrameDist + (mBorderSize/2.0);
   OSG::Real32 xoffset = (frame_width/2.0) + (mBorderSize/2.0);
   OSG::Real32 yoffset = (frame_height/2.0) + (mBorderSize/2.0);

   xoffset = xoffset * mDrawScale;
   yoffset = yoffset * mDrawScale;
   zoffset = zoffset * mDrawScale;

   // Create the xforms for each box.
   OSG::Matrix topm, bottomm, leftm, rightm;

   topm.setScale(mDrawScale);
   bottomm.setScale(mDrawScale);
   leftm.setScale(mDrawScale);
   rightm.setScale(mDrawScale);

   topm.setTranslate(0.0, yoffset, zoffset);
   bottomm.setTranslate(0.0, -yoffset, zoffset);
   leftm.setTranslate(-xoffset, 0.0, zoffset);
   rightm.setTranslate(xoffset, 0.0, zoffset);

   {
#if OSG_MAJOR_VERSION  <2
      OSG::CPEditor txe(top_xform.core(), OSG::Transform::MatrixFieldMask);
      OSG::CPEditor bxe(bottom_xform.core(), OSG::Transform::MatrixFieldMask);
      OSG::CPEditor lxe(left_xform.core(), OSG::Transform::MatrixFieldMask);
      OSG::CPEditor rxe(right_xform.core(), OSG::Transform::MatrixFieldMask);
#endif
      top_xform->setMatrix(topm);
      bottom_xform->setMatrix(bottomm);
      left_xform->setMatrix(leftm);
      right_xform->setMatrix(rightm);

      top_xform.node()->addChild(top.node());
      bottom_xform.node()->addChild(bottom.node());
      left_xform.node()->addChild(left.node());
      right_xform.node()->addChild(right.node());
   }

   // Create a node that will contain all geometry after the the merge op.
   OSG::NodePtr frame = OSG::Node::create();
   {
#if OSG_MAJOR_VERSION < 2
      OSG::CPEditor fe(
         frame, OSG::Node::CoreFieldMask | OSG::Node::ChildrenFieldMask
      );
#endif
      frame->setCore(OSG::Group::create());
      frame->addChild(top_xform.node());
      frame->addChild(bottom_xform.node());
      frame->addChild(left_xform.node());
      frame->addChild(right_xform.node());
   }

   // Merge all geometry into one node.
   OSG::MergeGraphOp merge;
   merge.traverse(frame);

   // Remove old geometry and add new.
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor fre(mFrameRoot, OSG::Node::ChildrenFieldMask);
#endif
   while (mFrameRoot->getNChildren() > 0)
   {
      mFrameRoot->subChild(0);
   }
   mFrameRoot->addChild(frame);
}

}

}
