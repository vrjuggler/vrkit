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

#ifndef _VRKIT_VIDEO_CAPTURE_PLUGIN_H_
#define _VRKIT_VIDEO_CAPTURE_PLUGIN_H_

#include <vrkit/plugin/Config.h>

#include <string>
#include <vector>
#include <boost/enable_shared_from_this.hpp>
#include <boost/signals/connection.hpp>

#include <OpenSG/OSGSwitch.h>
#include <OpenSG/OSGTransform.h>

#include <gadget/Type/PositionInterface.h>

#include <vrkit/ViewerPtr.h>
#include <vrkit/viewer/Plugin.h>
#include <vrkit/video/RecorderPtr.h>
#include <vrkit/util/DigitalCommand.h>


namespace vrkit
{

class VideoCapturePlugin
   : public viewer::Plugin
   , public boost::enable_shared_from_this<VideoCapturePlugin>
{
protected:
   VideoCapturePlugin(const plugin::Info& info);

public:
   static viewer::PluginPtr create(const plugin::Info& info)
   {
      return viewer::PluginPtr(new VideoCapturePlugin(info));
   }

   virtual ~VideoCapturePlugin();

   virtual std::string getDescription()
   {
      return std::string("Video Capture");
   }

   /**
    * Initializes this plug-in.
    *
    * @param viewer The VR Juggler application object.
    *
    * @return A pointer to this plug-in is returned.
    */
   virtual viewer::PluginPtr init(ViewerPtr viewer);

   /**
    *
    * @param viewer The VR Juggler application object.
    */
   virtual void contextInit(ViewerPtr viewer);

   /**
    *
    * @param viewer The VR Juggler application object.
    */
   virtual void contextPreDraw(ViewerPtr viewer);

   /**
    *
    * @param viewer The VR Juggler application object.
    */
   virtual void update(ViewerPtr viewer);

private:
   static std::string getElementType()
   {
      return "vrkit_video_capture_plugin";
   }

   void configure(jccl::ConfigElementPtr elt, ViewerPtr viewer);

   /** @name vrkit::video::Recorder Slots */
   //@{
   void recordingStarted();

   void recordingPaused();

   void recordingResumed();

   void recordingStopped();
   //@}

   void show(OSG::SwitchNodePtr switchNode);

   void hide(OSG::SwitchNodePtr switchNode);

   gadget::PositionInterface    mCameraProxy;
   video::RecorderPtr           mVideoRecorder;
   bool                         mShowViewFrame;
   bool                         mShowDebugFrame;

   OSG::SwitchNodePtr   mViewFrameNode;
   OSG::TransformRefPtr mViewFrameXform;
   OSG::SwitchNodePtr   mDebugFrameNode;
   OSG::TransformRefPtr mDebugFrameXform;

   std::vector<boost::signals::connection> mConnections;

   util::DigitalCommand mStartCmd;
   util::DigitalCommand mPauseCmd;
   util::DigitalCommand mResumeCmd;
   util::DigitalCommand mStopCmd;
};

}


#endif /* _VRKIT_VIDEO_CAPTURE_PLUGIN_H_ */
