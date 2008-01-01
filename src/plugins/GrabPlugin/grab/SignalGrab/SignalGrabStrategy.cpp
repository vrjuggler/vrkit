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

#include <vrkit/Config.h>

#include <algorithm>
#include <boost/bind.hpp>
#include <boost/assign/list_of.hpp>

#include <vrkit/plugin/Config.h>
#include <vrkit/InterfaceTrader.h>
#include <vrkit/WandInterface.h>
#include <vrkit/User.h>
#include <vrkit/Scene.h>
#include <vrkit/SceneObject.h>
#include <vrkit/Viewer.h>
#include <vrkit/Version.h>
#include <vrkit/scenedata/GrabSignalData.h>
#include <vrkit/scenedata/StatusPanelData.h>
#include <vrkit/plugin/Creator.h>
#include <vrkit/plugin/Info.h>
#include <vrkit/exceptions/PluginException.h>

#include "SignalGrabStrategy.h"


using namespace boost::assign;

static const vrkit::plugin::Info sInfo(
   "com.infiscape.grab", "SignalGrabStrategy",
   list_of(VRKIT_VERSION_MAJOR)(VRKIT_VERSION_MINOR)(VRKIT_VERSION_PATCH)
);
static vrkit::plugin::Creator<vrkit::grab::Strategy> sPluginCreator(
   boost::bind(&vrkit::SignalGrabStrategy::create, sInfo)
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

SignalGrabStrategy::SignalGrabStrategy(const plugin::Info& info)
   : grab::Strategy(info)
   , mChooseText("Choose object(s) to grab")
   , mGrabText("Grab object(s)")
   , mReleaseText("Release object(s)")
   , mGrabReleaseToggle(false)
   , mGrabbing(false)
{
   /* Do nothing. */ ;
}

SignalGrabStrategy::~SignalGrabStrategy()
{
   std::for_each(mConnections.begin(), mConnections.end(),
                 boost::bind(&boost::signals::connection::disconnect, _1));
}

grab::StrategyPtr SignalGrabStrategy::init(ViewerPtr viewer,
                                           grab_callback_t grabCallback,
                                           release_callback_t releaseCallback)
{
   mGrabCallback    = grabCallback;
   mReleaseCallback = releaseCallback;

   mGrabSignalData = viewer->getSceneObj()->getSceneData<GrabSignalData>();

   mConnections.push_back(
      mGrabSignalData->asyncRelease.connect(
         boost::bind(&SignalGrabStrategy::objectsReleased, this, _1)
      )
   );

   InterfaceTrader& if_trader = viewer->getUser()->getInterfaceTrader();
   mWandInterface = if_trader.getWandInterface();

   // Configure
   jccl::ConfigElementPtr cfg_elt =
      viewer->getConfiguration().getConfigElement(getElementType());

   if ( cfg_elt )
   {
      // Configure ourself.
      configure(cfg_elt);
   }

   return shared_from_this();
}

void SignalGrabStrategy::setFocus(ViewerPtr viewer, const bool focused)
{
   // If we have focus, we will try to update the staus panel to include our
   // commands.
   if ( focused )
   {
      ScenePtr scene = viewer->getSceneObj();
      StatusPanelDataPtr status_panel_data =
         scene->getSceneData<StatusPanelData>();

      // If choose button(s) is/are configured, we will update the status
      // panel to include that information.
      if ( mChooseBtn.isConfigured() )
      {
         status_panel_data->addControlText(mChooseBtn.toString(), mChooseText,
                                           1);
      }

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

      status_panel_data->removeControlText(mChooseBtn.toString(), mChooseText);
      status_panel_data->removeControlText(mGrabBtn.toString(), mGrabText);
      status_panel_data->removeControlText(mReleaseBtn.toString(),
                                           mReleaseText);
   }
}

void SignalGrabStrategy::update(ViewerPtr viewer)
{
   // The user wants to choose an object for later grabbing.
   if ( mChooseBtn() )
   {
      mGrabSignalData->choose();
   }
   // If the grab and release operations are activated using the same button
   // sequence, then we have to toggle back and forth between grabbing and
   // releasing objects.
   else if ( mGrabReleaseToggle )
   {
      // Just test the state of the grab command. Testing the state of the
      // release command would give the same result in this case.
      if ( mGrabBtn() )
      {
         // If the grab/release toggle has been activated, then we have to
         // look at mGrabbing to figure out which operation to perform.

         // If we are not currently grabbing anything, then this is a grab
         // operation.
         if ( ! mGrabbing )
         {
            grab();
         }
         // If we are currently grabbing something, then this is a release
         // operation.
         else
         {
            release();
         }
      }
   }
   else
   {
      // The user has requested to grab the currently selected objects.
      if ( mGrabBtn() )
      {
         grab();
      }
      // The user has requested to release grabbed objects.
      else if ( mReleaseBtn() )
      {
         release();
      }
   }
}

std::vector<SceneObjectPtr> SignalGrabStrategy::getGrabbedObjects()
{
   return mGrabbedObjects;
}

void SignalGrabStrategy::configure(jccl::ConfigElementPtr elt)
{
   vprASSERT(elt->getID() == getElementType());

   const unsigned int req_cfg_version(1);

   // Check for correct version of plugin configuration.
   if ( elt->getVersion() < req_cfg_version )
   {
      std::stringstream msg;
      msg << "Configuration of SignalGrabStrategy failed.  Required config "
          << "element version is " << req_cfg_version << ", but element '"
          << elt->getName() << "' is version " << elt->getVersion();
      throw PluginException(msg.str(), VRKIT_LOCATION);
   }

   const std::string choose_btn_prop("choose_button_nums");
   const std::string grab_btn_prop("grab_button_nums");
   const std::string release_btn_prop("release_button_nums");

   mChooseBtn.configure(elt->getProperty<std::string>(choose_btn_prop),
                        mWandInterface);
   mGrabBtn.configure(elt->getProperty<std::string>(grab_btn_prop),
                      mWandInterface);
   mReleaseBtn.configure(elt->getProperty<std::string>(release_btn_prop),
                         mWandInterface);

   // Determine if grab and release are activated using the same button
   // sequence. This indicates that the grab/release operation is a toggle
   // and must be handled differently than if the two operations are
   // separate.
   mGrabReleaseToggle = mGrabBtn == mReleaseBtn;
}

void SignalGrabStrategy::grab()
{
   // Emit the grab signal and get the collection of grabbed objects and
   // the intersection point.
   std::vector<SceneObjectPtr> grabbed_objs;
   gmtl::Point3f isect_pnt;
   mGrabSignalData->grab(grabbed_objs, isect_pnt);

   if ( ! grabbed_objs.empty() )
   {
      mGrabbing = true;
      mGrabbedObjects.insert(mGrabbedObjects.end(), grabbed_objs.begin(),
                             grabbed_objs.end());
      mGrabCallback(grabbed_objs, isect_pnt);
   }
}

void SignalGrabStrategy::release()
{
   // Emit the grab signal and get the collection of released objects.
   std::vector<SceneObjectPtr> released_objs;
   mGrabSignalData->release(released_objs);

   if ( ! released_objs.empty() )
   {
      std::vector<SceneObjectPtr>::iterator o;
      for ( o = released_objs.begin(); o != released_objs.end(); ++o )
      {
         mGrabbedObjects.erase(std::remove(mGrabbedObjects.begin(),
                                        mGrabbedObjects.end(), *o),
                               mGrabbedObjects.end());
      }

      mGrabbing = ! mGrabbedObjects.empty();
      mReleaseCallback(released_objs);
   }
}

void SignalGrabStrategy::
objectsReleased(const std::vector<SceneObjectPtr>& objs)
{
   std::vector<SceneObjectPtr>::const_iterator o;
   for ( o = objs.begin(); o != objs.end(); ++o )
   {
      mGrabbedObjects.erase(std::remove(mGrabbedObjects.begin(),
                                        mGrabbedObjects.end(), *o),
                            mGrabbedObjects.end());
   }

   mGrabbing = ! mGrabbedObjects.empty();
   mReleaseCallback(objs);
}

}
