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

#include <iostream>
#include <algorithm>
#include <boost/bind.hpp>
#include <boost/assign/list_of.hpp>

#include <gmtl/Coord.h>
#include <gmtl/Matrix.h>
#include <gmtl/Math.h>
#include <gmtl/Generate.h>
#include <gmtl/External/OpenSGConvert.h>

#include <vrkit/Viewer.h>
#include <vrkit/Scene.h>
#include <vrkit/User.h>
#include <vrkit/InterfaceTrader.h>
#include <vrkit/WandInterface.h>
#include <vrkit/Status.h>
#include <vrkit/Version.h>
#include <vrkit/Exception.h>
#include <vrkit/video/Recorder.h>
#include <vrkit/video/Encoder.h>
#include <vrkit/plugin/Creator.h>
#include <vrkit/plugin/Info.h>

#include "VideoCapturePlugin.h"


using namespace boost::assign;

static const vrkit::plugin::Info sInfo(
   "com.infiscape", "VideoCapturePlugin",
   list_of(VRKIT_VERSION_MAJOR)(VRKIT_VERSION_MINOR)(VRKIT_VERSION_PATCH)
);
static vrkit::plugin::Creator<vrkit::viewer::Plugin> sPluginCreator(
   boost::bind(&vrkit::VideoCapturePlugin::create, sInfo)
);

extern "C"
{

/** @name Plug-in Entry Points */
//@{
VRKIT_PLUGIN_API(const vrkit::plugin::Info*) getPluginInfo()
{
   return &sInfo;
}

VRKIT_PLUGIN_API(void) getPluginInterfaceVersion(vpr::Uint32& majorVer,
                                                 vpr::Uint32& minorVer)
{
   majorVer = VRKIT_PLUGIN_API_MAJOR;
   minorVer = VRKIT_PLUGIN_API_MINOR;
}

VRKIT_PLUGIN_API(vrkit::plugin::CreatorBase*) getCreator()
{
   return &sPluginCreator;
}
//@}
}

namespace vrkit
{

VideoCapturePlugin::VideoCapturePlugin(const plugin::Info& info)
   : viewer::Plugin(info)
   , mShowViewFrame(false)
   , mShowDebugFrame(false)
   , mViewFrameXform(OSG::NullFC)
   , mDebugFrameXform(OSG::NullFC)
{
   /* Do nothing. */ ;
}

viewer::PluginPtr VideoCapturePlugin::init(ViewerPtr viewer)
{
   mVideoRecorder = video::Recorder::create()->init();
   mVideoRecorder->setDrawScaleFactor(viewer->getDrawScaleFactor());

   jccl::ConfigElementPtr elt =
      viewer->getConfiguration().getConfigElement(getElementType());

   if ( elt )
   {
      configure(elt, viewer);
   }

#if 1
   const video::Encoder::container_format_list_t& formats =
      mVideoRecorder->getAvailableFormats();

   // Debug code for printing out all choices. Should get relocated to Encoder?
   std::cout << "Encoding Format Choices" << std::endl;
   std::cout << formats.size() << " choices.." << std::endl;
   std::cout << "-------------------------------------" << std::endl;
   typedef video::Encoder::container_format_list_t::const_iterator iter_type;
   for ( iter_type i = formats.begin(); i != formats.end(); ++i )
   {
      std::cout << (*i).mFormatLongName << std::endl;
      std::cout << "  id         : " << (*i).mFormatName << std::endl;
      std::cout << "  encoder    : " << (*i).mEncoderName << std::endl;
      std::cout << "  codecs     :" << std::endl;
      if ( 0 == (*i).mCodecList.size() )
      {
         std::cout << "               default" << std::endl;
      }
      else
      {
         typedef video::Encoder::codec_list_t::const_iterator codec_iter_type;
         for ( codec_iter_type j = (*i).mCodecList.begin();
               j != (*i).mCodecList.end();
               ++j )
         {
            std::cout << "               " << *j << std::endl;
         }
      }
      std::cout << "  extensions :" << std::endl;
      typedef std::vector<std::string>::const_iterator ext_iter_type;
      for ( ext_iter_type j = (*i).mFileExtensions.begin();
            j != (*i).mFileExtensions.end();
            ++j )
      {
         std::cout << "               " << *j << std::endl;
      }
      std::cout << "-------------------------------------" << std::endl;
   }
#endif

   OSG::GroupNodePtr decorator_root =
      viewer->getSceneObj()->getDecoratorRoot();
   OSG::NodeRefPtr decorator_node(decorator_root.node());

   if ( mShowViewFrame )
   {
      mViewFrameNode = OSG::Switch::create();

      OSG::TransformNodePtr frame_xform(OSG::Transform::create());
      mViewFrameXform = frame_xform.core();

      // We have to put a transform node above the view frame node so that
      // it can be moved relative to the camera attachment transformation.
      OSG::NodeRefPtr frame_node(mViewFrameNode.node());
      OSG::beginEditCP(frame_node, OSG::Node::ChildrenFieldMask);
         frame_node->addChild(frame_xform.node());
      OSG::endEditCP(frame_node, OSG::Node::ChildrenFieldMask);

      OSG::beginEditCP(frame_xform.node(), OSG::Node::ChildrenFieldMask);
         frame_xform.node()->addChild(mVideoRecorder->getFrame());
      OSG::endEditCP(frame_xform.node(), OSG::Node::ChildrenFieldMask);

      OSG::beginEditCP(decorator_node, OSG::Node::ChildrenFieldMask);
         decorator_node->addChild(mViewFrameNode.node());
      OSG::endEditCP(decorator_node, OSG::Node::ChildrenFieldMask);
   }

   if ( mShowDebugFrame )
   {
      // Create the test plane to put the texture on.
      mDebugFrameNode = OSG::Switch::create();

      OSG::NodePtr frame_xform_node = OSG::Node::create();
      OSG::beginEditCP(frame_xform_node, OSG::Node::CoreFieldMask);
         frame_xform_node->setCore(mDebugFrameXform);
      OSG::endEditCP(frame_xform_node, OSG::Node::CoreFieldMask);

      // We have to put a transform node above the view frame node so that
      // it can be positioned according to the user configuration.
      OSG::NodeRefPtr debug_node(mDebugFrameNode.node());
      OSG::beginEditCP(debug_node, OSG::Node::ChildrenFieldMask);
         debug_node->addChild(frame_xform_node);
      OSG::endEditCP(debug_node, OSG::Node::ChildrenFieldMask);

      OSG::beginEditCP(frame_xform_node, OSG::Node::ChildrenFieldMask);
         frame_xform_node->addChild(mVideoRecorder->getDebugPlane());
      OSG::endEditCP(frame_xform_node, OSG::Node::ChildrenFieldMask);

      OSG::beginEditCP(decorator_node, OSG::Node::ChildrenFieldMask);
         decorator_node->addChild(mDebugFrameNode.node());
      OSG::endEditCP(decorator_node, OSG::Node::ChildrenFieldMask);
   }

   mConnections.push_back(
      mVideoRecorder->recordingStarted().connect(
         boost::bind(&VideoCapturePlugin::recordingStarted, this)
      )
   );
   mConnections.push_back(
      mVideoRecorder->recordingPaused().connect(
         boost::bind(&VideoCapturePlugin::recordingPaused, this)
      )
   );
   mConnections.push_back(
      mVideoRecorder->recordingResumed().connect(
         boost::bind(&VideoCapturePlugin::recordingResumed, this)
      )
   );
   mConnections.push_back(
      mVideoRecorder->recordingStopped().connect(
         boost::bind(&VideoCapturePlugin::recordingStopped, this)
      )
   );

   // Ensure that this is called after anythin that effects getScene().
   // VR Juggler normally calls this each frame before rendering.
   mVideoRecorder->setSceneRoot(viewer->getScene());

   return shared_from_this();
}

VideoCapturePlugin::~VideoCapturePlugin()
{
   if ( mVideoRecorder->isRecording() )
   {
      mVideoRecorder->endRecording();
   }

   std::for_each(mConnections.begin(), mConnections.end(),
                 boost::bind(&boost::signals::connection::disconnect, _1));
}

void VideoCapturePlugin::contextInit(ViewerPtr viewer)
{
   // Context-specific data. Should be one copy per context.
   mVideoRecorder->contextInit(viewer->getContextData().mWin);
}

void VideoCapturePlugin::contextPreDraw(ViewerPtr viewer)
{
   OSG::Matrix4f head_trans;
   gmtl::set(head_trans, mCameraProxy->getData(viewer->getDrawScaleFactor()));

   OSG::beginEditCP(mViewFrameXform, OSG::Transform::MatrixFieldMask);
      mViewFrameXform->setMatrix(head_trans);
   OSG::endEditCP(mViewFrameXform, OSG::Transform::MatrixFieldMask);

   const Viewer::base_type::context_data& c_data = viewer->getContextData();
   c_data.mRenderAction->setWindow(c_data.mWin.getCPtr());
   mVideoRecorder->render(c_data.mRenderAction, head_trans);
}

void VideoCapturePlugin::update(ViewerPtr viewer)
{
   if ( ! mVideoRecorder->isRecording() && mStartCmd() )
   {
      mVideoRecorder->startRecording();
   }
   else if ( mVideoRecorder->isRecording() && mPauseCmd() )
   {
      mVideoRecorder->pause();
   }
   else if ( mVideoRecorder->isPaused() && mResumeCmd() )
   {
      mVideoRecorder->resume();
   }
   else if ( mVideoRecorder->isRecording() && mStopCmd() )
   {
      mVideoRecorder->endRecording();
   }
}

void VideoCapturePlugin::configure(jccl::ConfigElementPtr elt,
                                   ViewerPtr viewer)
{
   const std::string output_file_prop("output_file");
   const std::string encoder_prop("encoder");
   const std::string codec_prop("codec");
   const std::string resolution_prop("resolution");
   const std::string fov_prop("fov");
   const std::string fps_prop("fps");
   const std::string stereo_recording_prop("stereo_recording");
   const std::string camera_proxy_prop("camera_proxy");
   const std::string show_view_frame_prop("show_view_frame");
   const std::string view_frame_dist_prop("view_frame_distance");
   const std::string view_frame_border_size_prop("view_frame_border_size");
   const std::string show_debug_frame_prop("show_debug_frame");
   const std::string start_command_exp_prop("start_command_exp");
   const std::string pause_command_exp_prop("pause_command_exp");
   const std::string resume_command_exp_prop("resume_command_exp");
   const std::string stop_command_exp_prop("stop_command_exp");

   std::string format_name("avi");

   const std::string output_file =
      elt->getProperty<std::string>(output_file_prop);

   if ( ! output_file.empty() )
   {
      mVideoRecorder->setFilename(output_file);
      const std::string::size_type dot_loc = output_file.rfind(".");
      format_name = output_file.substr(dot_loc + 1);
   }
   else
   {
      mVideoRecorder->setFilename("vrkit_movie.avi");
   }

//   const video::Encoder::container_format_list_t& formats =
//      mVideoRecorder->getAvailableFormats();

   // TODO: Validate that the named encoder is available.
   std::string encoder_name = elt->getProperty<std::string>(encoder_prop);

   // TODO: Validate that the named codec is available from the named encoder.
   std::string codec_name = elt->getProperty<std::string>(codec_prop);

   int width  = elt->getProperty<int>(resolution_prop, 0);
   int height = elt->getProperty<int>(resolution_prop, 1);

   if ( width <= 0 )
   {
      VRKIT_STATUS << "WARNING: Invalid video resolution width " << width
                   << " given; using 512." << std::endl;
      width = 512;
   }

   if ( height <= 0 )
   {
      VRKIT_STATUS << "WARNING: Invalid video resolution height " << height
                   << " given; using 512." << std::endl;
      height = 512;
   }

   mVideoRecorder->setFrameSize(width, height);

   float fov = elt->getProperty<float>(fov_prop);

   if ( fov <= 0.0f )
   {
      VRKIT_STATUS << "WARNING: Invalid field of view " << fov
                   << " given; using 60 degrees." << std::endl;
      fov = 60.0f;
   }

   mVideoRecorder->setFov(fov);

   int fps = elt->getProperty<int>(fps_prop);

   if ( fps <= 0 )
   {
      VRKIT_STATUS << "WARNING: Invalid frames per second setting " << fps
                   << " fps given; using 60 fps." << std::endl;
      fps = 60;
   }

   mVideoRecorder->setFramesPerSecond(fps);
   mVideoRecorder->setStereo(elt->getProperty<bool>(stereo_recording_prop));

   std::string camera_proxy = elt->getProperty<std::string>(camera_proxy_prop);

   if ( camera_proxy.empty() )
   {
      VRKIT_STATUS << "No camera attachment provided; using the user's head."
                   << std::endl;
      camera_proxy = viewer->getUser()->getHeadProxy()->getName();
   }

   mCameraProxy.init(camera_proxy);

   mShowViewFrame  = elt->getProperty<bool>(show_view_frame_prop);
   mShowDebugFrame = elt->getProperty<bool>(show_debug_frame_prop);

   if ( mShowDebugFrame )
   {
      const std::string frame_trans_prop("debug_frame_translation");
      const std::string frame_rot_prop("debug_frame_rotation");

      gmtl::Coord3fXYZ vp_coord;
      vp_coord.pos().set(
         elt->getProperty<float>(frame_trans_prop, 0),
         elt->getProperty<float>(frame_trans_prop, 1),
         elt->getProperty<float>(frame_trans_prop, 2)
      );
      vp_coord.rot().set(
         gmtl::Math::deg2Rad(elt->getProperty<float>(frame_rot_prop, 0)),
         gmtl::Math::deg2Rad(elt->getProperty<float>(frame_rot_prop, 1)),
         gmtl::Math::deg2Rad(elt->getProperty<float>(frame_rot_prop, 2))
      );

      OSG::Matrix xform_osg;
      gmtl::set(xform_osg, gmtl::make<gmtl::Matrix44f>(vp_coord));

      mDebugFrameXform = OSG::Transform::create();

      OSG::beginEditCP(mDebugFrameXform, OSG::Transform::MatrixFieldMask);
         mDebugFrameXform->setMatrix(xform_osg);
      OSG::endEditCP(mDebugFrameXform, OSG::Transform::MatrixFieldMask);

      mVideoRecorder->setDebugFrameDistance(
         elt->getProperty<float>(view_frame_dist_prop)
      );
      mVideoRecorder->setDebugFrameBorderWidth(
         elt->getProperty<float>(view_frame_border_size_prop)
      );
   }

   video::EncoderManager::video_encoder_format_t config;
   config.mEncoderName     = encoder_name;
   config.mContainerFormat = format_name;
   config.mCodec           = codec_name;
   mVideoRecorder->setFormat(config);

   WandInterfacePtr wand_if =
      viewer->getUser()->getInterfaceTrader().getWandInterface();
   mStartCmd.configure(elt->getProperty<std::string>(start_command_exp_prop),
                       wand_if);
   mPauseCmd.configure(elt->getProperty<std::string>(pause_command_exp_prop),
                       wand_if);
   mResumeCmd.configure(elt->getProperty<std::string>(resume_command_exp_prop),
                        wand_if);
   mStopCmd.configure(elt->getProperty<std::string>(stop_command_exp_prop),
                      wand_if);
}

void VideoCapturePlugin::recordingStarted()
{
   if ( mShowViewFrame )
   {
      show(mViewFrameNode);
   }

   if ( mShowDebugFrame )
   {
      show(mDebugFrameNode);
   }
}

void VideoCapturePlugin::recordingPaused()
{
   if ( mShowViewFrame )
   {
      hide(mViewFrameNode);
   }

   if ( mShowDebugFrame )
   {
      hide(mDebugFrameNode);
   }
}

void VideoCapturePlugin::recordingResumed()
{
   if ( mShowViewFrame )
   {
      show(mViewFrameNode);
   }

   if ( mShowDebugFrame )
   {
      show(mDebugFrameNode);
   }
}

void VideoCapturePlugin::recordingStopped()
{
   if ( mShowViewFrame )
   {
      hide(mViewFrameNode);
   }

   if ( mShowDebugFrame )
   {
      hide(mDebugFrameNode);
   }
}

void VideoCapturePlugin::show(OSG::SwitchNodePtr switchNode)
{
   OSG::beginEditCP(switchNode, OSG::Switch::ChoiceFieldMask);
      switchNode->setChoice(OSG::Switch::ALL);
   OSG::endEditCP(switchNode, OSG::Switch::ChoiceFieldMask);
}

void VideoCapturePlugin::hide(OSG::SwitchNodePtr switchNode)
{
   OSG::beginEditCP(switchNode, OSG::Switch::ChoiceFieldMask);
      switchNode->setChoice(OSG::Switch::NONE);
   OSG::endEditCP(switchNode, OSG::Switch::ChoiceFieldMask);
}

}
