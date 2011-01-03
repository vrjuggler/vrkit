// vrkit is (C) Copyright 2005-2011
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

#include <vrkit/Config.h>

#include <boost/bind.hpp>
#include <boost/assign/list_of.hpp>

#include <gmtl/External/OpenSGConvert.h>

#include <vpr/Util/Assert.h>
#include <jccl/Config/ConfigElement.h>

#include <vrkit/plugin/Config.h>
#include <vrkit/Scene.h>
#include <vrkit/User.h>
#include <vrkit/Viewer.h>
#include <vrkit/InterfaceTrader.h>
#include <vrkit/WandInterface.h>
#include <vrkit/SceneObject.h>
#include <vrkit/Version.h>
#include <vrkit/scenedata/EventData.h>
#include <vrkit/scenedata/StatusPanelData.h>
#include <vrkit/plugin/Creator.h>
#include <vrkit/plugin/Info.h>
#include <vrkit/exceptions/PluginException.h>

#include "SingleObjectGrabStrategy.h"


using namespace boost::assign;

static const vrkit::plugin::Info sInfo(
   "com.infiscape.grab", "SingleObjectGrabStrategy",
   list_of(VRKIT_VERSION_MAJOR)(VRKIT_VERSION_MINOR)(VRKIT_VERSION_PATCH)
);
static vrkit::plugin::Creator<vrkit::grab::Strategy> sPluginCreator(
   boost::bind(&vrkit::SingleObjectGrabStrategy::create, sInfo)
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
   majorVer = VRKIT_GRAB_STRATEGY_PLUGIN_API_MAJOR;
   minorVer = VRKIT_GRAB_STRATEGY_PLUGIN_API_MINOR;
}

VRKIT_PLUGIN_API(vrkit::plugin::CreatorBase*) getGrabStrategyCreator()
{
   return &sPluginCreator;
}
//@}

}

namespace vrkit
{

SingleObjectGrabStrategy::SingleObjectGrabStrategy(const plugin::Info& info)
   : grab::Strategy(info)
   , mGrabText("Grab")
   , mReleaseText("Release")
   , mIntersecting(false)
   , mGrabbing(false)
{
   /* Do nothing. */ ;
}

SingleObjectGrabStrategy::~SingleObjectGrabStrategy()
{
   mIsectConnection.disconnect();
   mDeIsectConnection.disconnect();
   mGrabbedObjConnection.disconnect();
}

grab::StrategyPtr SingleObjectGrabStrategy::
init(ViewerPtr viewer, grab_callback_t grabCallback,
     release_callback_t releaseCallback)
{
   mGrabCallback    = grabCallback;
   mReleaseCallback = releaseCallback;

   EventDataPtr event_data = viewer->getSceneObj()->getSceneData<EventData>();

   // Connect the intersection signal to our slot.
   mIsectConnection =
      event_data->objectIntersected.connect(
         0, boost::bind(&SingleObjectGrabStrategy::objectIntersected, this,
                        _1, _2)
      );

   // Connect the de-intersection signal to our slot.
   mDeIsectConnection =
      event_data->objectDeintersected.connect(
         0, boost::bind(&SingleObjectGrabStrategy::objectDeintersected,
                        this, _1)
      );

   InterfaceTrader& if_trader = viewer->getUser()->getInterfaceTrader();
   mWandInterface = if_trader.getWandInterface();

   // Configure
   std::string elt_type_name = getElementType();
   jccl::ConfigElementPtr cfg_elt =
      viewer->getConfiguration().getConfigElement(elt_type_name);

   if ( cfg_elt )
   {
      // Configure ourself.
      configure(cfg_elt);
   }

   return shared_from_this();
}

void SingleObjectGrabStrategy::setFocus(ViewerPtr viewer, const bool focused)
{
   // If we have focus, we will try to update the staus panel to include our
   // commands.
   if ( focused )
   {
      ScenePtr scene = viewer->getSceneObj();
      StatusPanelDataPtr status_panel_data =
         scene->getSceneData<StatusPanelData>();

      // If grab button(s) is/are configured, we will update the status
      // panel to include that information.
      if ( mGrabBtn.isConfigured() )
      {
         status_panel_data->addControlText(mGrabBtn.toString(), mGrabText, 1);
      }

      // If release button(s) is/are configured, we will update the status
      // panel to include that information.
      if ( mReleaseBtn.isConfigured() )
      {
         status_panel_data->addControlText(mReleaseBtn.toString(),
                                           mReleaseText, 1);
      }
   }
   else
   {
      ScenePtr scene = viewer->getSceneObj();
      StatusPanelDataPtr status_panel_data =
         scene->getSceneData<StatusPanelData>();

      status_panel_data->removeControlText(mGrabBtn.toString(), mGrabText);
      status_panel_data->removeControlText(mReleaseBtn.toString(),
                                           mReleaseText);
   }
}

void SingleObjectGrabStrategy::update(ViewerPtr viewer)
{
   // If we are intersecting an object but not grabbing it, the grab
   // button has just been pressed, and the intersected object is grabbable,
   // then we can grab the intersected object.
   if ( mIntersecting && ! mGrabbing && mIntersectedObj->isGrabbable() &&
        mGrabBtn() )
   {
      mGrabbing = true;
      mGrabbedObj = mIntersectedObj;

      // Connect the grabbable object state change signal to our slot.
      mGrabbedObjConnection =
         mGrabbedObj->grabbableStateChanged().connect(
            boost::bind(&SingleObjectGrabStrategy::grabbableObjStateChanged,
                        this, _1)
         );

      std::vector<SceneObjectPtr> objs(1, mGrabbedObj);
      mGrabCallback(objs, mIntersectPoint);
   }
   // If we are grabbing an object and the release button has just been
   // pressed, then release the grabbed object.
   else if ( mGrabbing && mReleaseBtn() )
   {
      releaseGrabbedObject();
   }
}

std::vector<SceneObjectPtr> SingleObjectGrabStrategy::getGrabbedObjects()
{
   std::vector<SceneObjectPtr> objs;

   if ( mGrabbedObj )
   {
      objs.push_back(mGrabbedObj);
   }

   return objs;
}

void SingleObjectGrabStrategy::configure(jccl::ConfigElementPtr elt)
{
   vprASSERT(elt->getID() == getElementType());

   const unsigned int req_cfg_version(1);

   // Check for correct version of plugin configuration.
   if ( elt->getVersion() < req_cfg_version )
   {
      std::stringstream msg;
      msg << "Configuration of SingleObjectGrabStrategy failed.  Required "
          << "config element version is " << req_cfg_version
          << ", but element '" << elt->getName() << "' is version "
          << elt->getVersion();
      throw PluginException(msg.str(), VRKIT_LOCATION);
   }

   const std::string grab_btn_prop("grab_button_nums");
   const std::string release_btn_prop("release_button_nums");

   mGrabBtn.configure(elt->getProperty<std::string>(grab_btn_prop),
                      mWandInterface);
   mReleaseBtn.configure(elt->getProperty<std::string>(release_btn_prop),
                         mWandInterface);
}

event::ResultType
SingleObjectGrabStrategy::objectIntersected(SceneObjectPtr obj,
                                            const gmtl::Point3f& pnt)
{
   if ( ! mGrabbing )
   {
      // If we intersected a grabbable object.
      if ( obj->isGrabbable() )
      {
         mIntersectedObj = obj;
         mIntersectPoint = pnt;
         mIntersecting = true;
      }
   }
   else
   {
      return event::DONE;
   }

   return event::CONTINUE;
}

event::ResultType
SingleObjectGrabStrategy::objectDeintersected(SceneObjectPtr obj)
{
   if ( mGrabbing )
   {
      return event::DONE;
   }

   mIntersecting = false;
   mIntersectedObj = SceneObjectPtr();

   return event::CONTINUE;
}

void SingleObjectGrabStrategy::releaseGrabbedObject()
{
   // Update our state to reflect that we are no longer grabbing an object.
   mGrabbing = false;

   // Disconnect mGrabbedObjConnection so that any state changes to
   // mGrabbedObj do not propagate back to us. We are no longer interested in
   // changes since we have released the object.
   mGrabbedObjConnection.disconnect();

   std::vector<SceneObjectPtr> objs(1, mGrabbedObj);

   // Finish off the object release operation by clearing our reference to
   // the formerly grabbed object.
   mGrabbedObj = SceneObjectPtr();

   mReleaseCallback(objs);
}

void SingleObjectGrabStrategy::grabbableObjStateChanged(SceneObjectPtr obj)
{
   // If we are currently grabbing obj and its grabbable state has changed so
   // that it is no longer grabbable, we must release it.
   if ( mGrabbing && mGrabbedObj == obj && ! mGrabbedObj->isGrabbable() )
   {
      releaseGrabbedObject();
   }
}

}
