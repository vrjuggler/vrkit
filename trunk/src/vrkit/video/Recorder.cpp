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

#include <algorithm>
#include <boost/bind.hpp>

#include <OpenSG/OSGGeometry.h>
#include <OpenSG/OSGGeoPropPtrs.h>
#include <OpenSG/OSGImage.h>
#include <OpenSG/OSGMergeGraphOp.h>
#include <OpenSG/OSGGraphOpSeq.h>
#include <OpenSG/OSGSimpleGeometry.h>
#include <OpenSG/OSGSimpleMaterial.h>

#include <vrkit/video/EncoderManager.h>
#include <vrkit/video/CameraFBO.h>
#include <vrkit/video/Recorder.h>


namespace vrkit
{

namespace video
{

Recorder::Recorder()
   : mCamera()
   , mVideoEncoder()
   , mStereoImageStorage(OSG::NullFC)
   , mTransform(OSG::NullFC)
   , mFrameRoot(OSG::NullFC)
   , mEyeOffset(0.5)
   , mBorderSize(2.0)
   , mFrameDist(50.0)
   , mStereo(false)
   , mRecording(false)
   , mPaused(false)
   , mDrawScale(1.0)
{
   ;
}

RecorderPtr Recorder::create()
{
   return RecorderPtr(new Recorder);
}

Recorder::~Recorder()
{
   std::for_each(mEncoderConnections.begin(), mEncoderConnections.end(),
                 boost::bind(&boost::signals::connection::disconnect, _1));
}

void Recorder::contextInit(OSG::WindowPtr window)
{
   mCamera->setWindow(window);
}

RecorderPtr Recorder::init()
{
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

   mStereoImageStorage = OSG::Image::create();

   // TODO: Do not hard code the use of CameraFBO.
   mCamera = CameraFBO::create()->init();

   assert(mCamera.get() != NULL);

   mVideoEncoder = EncoderManager::create()->init();

   mEncoderConnections.push_back(
      mVideoEncoder->encodingStarted().connect(
         boost::bind(&Recorder::encodingStarted, this)
      )
   );
   mEncoderConnections.push_back(
      mVideoEncoder->encodingPaused().connect(
         boost::bind(&Recorder::encodingPaused, this)
      )
   );
   mEncoderConnections.push_back(
      mVideoEncoder->encodingResumed().connect(
         boost::bind(&Recorder::encodingResumed, this)
      )
   );
   mEncoderConnections.push_back(
      mVideoEncoder->encodingStopped().connect(
         boost::bind(&Recorder::encodingStopped, this)
      )
   );

   return shared_from_this();
}

void Recorder::setSceneRoot(OSG::NodePtr root)
{
   mCamera->setSceneRoot(root);
}

void Recorder::setFilename(const std::string& filename)
{
   mVideoEncoder->setFilename(filename);
}

void Recorder::setFramesPerSecond(const OSG::UInt32 framesPerSecond)
{
   mVideoEncoder->setFramesPerSecond(framesPerSecond);
}

void Recorder::setFormat(const EncoderManager::video_encoder_format_t& format)
{
   mVideoEncoder->setFormat(format);
}

const Encoder::container_format_list_t& Recorder::getAvailableFormats() const
{
   return mVideoEncoder->getAvailableFormats();
}

void Recorder::setStereo(const bool stereo)
{
   mStereo = stereo;
   mVideoEncoder->setStereo(stereo);
}

void Recorder::setTravMask(const OSG::UInt32 value)
{
   mCamera->setTravMask(value);
}

void Recorder::setInterocularDistance(const OSG::Real32 interocular)
{
   mEyeOffset = interocular / 2.0f;
}

void Recorder::setFrameSize(const OSG::UInt32 width, const OSG::UInt32 height)
{
   mCamera->setSize(width, height);
   mVideoEncoder->setSize(width, height);

   generateDebugFrame(); //XXX
}

void Recorder::setCameraPos(const OSG::Matrix& camPos)
{
   mCamera->setPosition(camPos);
}

void Recorder::startRecording()
{
   if ( ! isRecording() )
   {
      // Ensure that recording actually started.
      if (mVideoEncoder->record())
      {
         mCamera->setPixelFormat(mVideoEncoder->getPixelFormat());

         if( mStereo )
         {
            OSG::beginEditCP(mStereoImageStorage);
               mStereoImageStorage->set(mVideoEncoder->getPixelFormat(),
                                        mCamera->getWidth() * 2,
                                        mCamera->getHeight());
            OSG::beginEditCP(mStereoImageStorage);
         }
      }
   }
}

void Recorder::pause()
{
   if( isRecording() && ! isPaused() )
   {
      mVideoEncoder->pause();
   }
}

void Recorder::resume()
{
   if( isRecording() && isPaused() )
   {
      mVideoEncoder->resume();
   }
}

void Recorder::endRecording()
{
   if( isRecording() )
   {
      mVideoEncoder->stop();
   }
}

bool Recorder::isRecording() const
{
   return mRecording;
}

bool Recorder::isPaused() const
{
   return mPaused;
}

void Recorder::setAspect(const OSG::Real32 aspect)
{
   mCamera->setAspect(aspect);
}

void Recorder::setFov(const OSG::Real32 fov)
{
   mCamera->setFov(OSG::osgdegree2rad(fov));
}

void Recorder::setNearFar(const OSG::Real32 nearVal, const OSG::Real32 farVal)
{
   mCamera->setNearFar(nearVal, farVal);
}

void Recorder::render(OSG::RenderAction* ra, const OSG::Matrix& camPos)
{
   if (!isRecording() || isPaused() )
   {
      return;
   }

   // If we are rendering stereo, then offset the camera position.
   if (mStereo)
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

      OSG::beginEditCP(mStereoImageStorage);
         mStereoImageStorage->setSubData(
            0, 0, 0, width, height, 1, mCamera->getLeftEyeImage()->getData()
         );
         mStereoImageStorage->setSubData(
            width, 0, 0, width, height, 1,
            mCamera->getRightEyeImage()->getData()
         );
      OSG::endEditCP(mStereoImageStorage);

      mVideoEncoder->writeFrame(mStereoImageStorage);
   }
   else
   {
      setCameraPos(camPos);

      mCamera->renderLeftEye(ra);

      mVideoEncoder->writeFrame(mCamera->getLeftEyeImage());

   }
}

OSG::NodePtr Recorder::getDebugPlane() const
{
   // Create material for left eye.
   OSG::SimpleMaterialPtr left_mat = OSG::SimpleMaterial::create();
   OSG::beginEditCP(left_mat);
      left_mat->addChunk(mCamera->getLeftTexture());
      left_mat->setSpecular(OSG::Color3f(0.7f, 0.7f, 0.7f));
      left_mat->setDiffuse(OSG::Color3f(0.22f, 0.2f, 0.2f));
   OSG::endEditCP(left_mat);

   // Create material for right eye.
   OSG::SimpleMaterialPtr right_mat = OSG::SimpleMaterial::create();
   OSG::beginEditCP(right_mat);
      right_mat->addChunk(mCamera->getRightTexture());
      right_mat->setSpecular(OSG::Color3f(0.7f, 0.7f, 0.7f));
      right_mat->setDiffuse(OSG::Color3f(0.22f, 0.2f, 0.2f));
   OSG::endEditCP(right_mat);

   // Create geometry for left eye.
   OSG::GeometryPtr left_geom = OSG::makePlaneGeo(5, 5, 2, 2);
   OSG::NodePtr left_node = OSG::Node::create();
   OSG::beginEditCP(left_geom);
      left_geom->setMaterial(left_mat);
   OSG::endEditCP(left_geom);
   OSG::beginEditCP(left_node);
      left_node->setCore(left_geom);
   OSG::endEditCP(left_node);

   // Create geometry for right eye.
   OSG::GeometryPtr right_geom = OSG::makePlaneGeo(5, 5, 2, 2);
   OSG::NodePtr right_node = OSG::Node::create();
   OSG::beginEditCP(right_geom);
      right_geom->setMaterial(right_mat);
   OSG::endEditCP(right_geom);
   OSG::beginEditCP(right_node);
      right_node->setCore(right_geom);
   OSG::endEditCP(right_node);

   // Create the xforms for each eye's geometry.
   OSG::Matrix leftm, rightm;
   leftm.setTranslate(-2.5, 2.5, 0.0);
   rightm.setTranslate(2.5, 2.5, 0.0);
   leftm.setScale(mDrawScale);
   rightm.setScale(mDrawScale);

   // Create the transform nodes for each eye's geometry.
   OSG::TransformNodePtr left_xform = OSG::TransformNodePtr::create();
   OSG::TransformNodePtr right_xform = OSG::TransformNodePtr::create();
   OSG::beginEditCP(left_xform);
   OSG::beginEditCP(right_xform);
      left_xform->setMatrix(leftm);
      right_xform->setMatrix(rightm);
      left_xform.node()->addChild(left_node);
      right_xform.node()->addChild(right_node);
   OSG::endEditCP(left_xform);
   OSG::endEditCP(right_xform);

   OSG::GroupPtr group = OSG::Group::create();
   OSG::NodePtr group_node = OSG::Node::create();
   OSG::beginEditCP(group_node);
      group_node->setCore(group);
      group_node->addChild(left_xform.node());
      group_node->addChild(right_xform.node());
   OSG::endEditCP(group_node);
   return group_node;
}

OSG::NodePtr Recorder::getFrame() const
{
   return mFrameRoot;
}

void Recorder::setDrawScaleFactor(float scale)
{
   mDrawScale = scale;
   generateDebugFrame();
}

void Recorder::setDebugFrameBorderWidth(float value)
{
   mBorderSize = value;
   generateDebugFrame();
}

void Recorder::setDebugFrameDistance(float value)
{
   mFrameDist = value;
   generateDebugFrame();
}

// XXX: This has not been updated to behave correctly in stereo mode.
void Recorder::generateDebugFrame()
{
   // The size of the internal frame.
   OSG::Real32 frame_height = 2.0 * (OSG::osgtan(mCamera->getFov()/2.0) * mFrameDist);
   OSG::Real32 frame_width = frame_height * mCamera->getAspect();

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

void Recorder::encodingStarted()
{
   mRecordingStarted();
   mRecording = true;
}

void Recorder::encodingPaused()
{
   mRecordingPaused();
   mPaused = true;
}

void Recorder::encodingResumed()
{
   mRecordingResumed();
   mPaused = false;
}

void Recorder::encodingStopped()
{
   mRecordingStopped();
   mRecording = false;
   mPaused    = false;
}

}

}
