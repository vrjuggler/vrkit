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

#include <vrkit/Config.h>

#include <functional>
#include <boost/bind.hpp>
#include <boost/assign/list_of.hpp>

#include <gmtl/External/OpenSGConvert.h>

#include <vpr/Util/Assert.h>
#include <jccl/Config/ConfigElement.h>

#include <vrkit/plugin/Config.h>
#include <vrkit/scenedata/EventData.h>
#include <vrkit/Scene.h>
#include <vrkit/User.h>
#include <vrkit/Viewer.h>
#include <vrkit/InterfaceTrader.h>
#include <vrkit/WandInterface.h>
#include <vrkit/SceneObject.h>
#include <vrkit/Version.h>
#include <vrkit/scenedata/StatusPanelData.h>
#include <vrkit/plugin/Creator.h>
#include <vrkit/plugin/Info.h>
#include <vrkit/exceptions/PluginException.h>

#include "MultiObjectGrabStrategy.h"


using namespace boost::assign;

static const vrkit::plugin::Info sInfo(
   "com.infiscape.grab", "MultiObjectGrabStrategy",
   list_of(VRKIT_VERSION_MAJOR)(VRKIT_VERSION_MINOR)(VRKIT_VERSION_PATCH)
);
static vrkit::plugin::Creator<vrkit::grab::Strategy> sPluginCreator(
   boost::bind(&vrkit::MultiObjectGrabStrategy::create, sInfo)
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

MultiObjectGrabStrategy::MultiObjectGrabStrategy(const plugin::Info& info)
   : grab::Strategy(info)
   , mChooseText("Choose object to grab")
   , mGrabText("Grab object(s)")
   , mReleaseText("Release object(s)")
   , mGrabbing(false)
{
   /* Do nothing. */ ;
}

MultiObjectGrabStrategy::~MultiObjectGrabStrategy()
{
   std::for_each(
      mObjConnections.begin(), mObjConnections.end(),
      boost::bind(&boost::signals::connection::disconnect,
                  boost::bind(&obj_conn_map_t::value_type::second, _1))
   );
   std::for_each(mIsectConnections.begin(), mIsectConnections.end(),
                 boost::bind(&boost::signals::connection::disconnect, _1));
}

grab::StrategyPtr MultiObjectGrabStrategy::
init(ViewerPtr viewer, grab_callback_t grabCallback,
     release_callback_t releaseCallback)
{
   mEventData = viewer->getSceneObj()->getSceneData<EventData>();

   mGrabCallback    = grabCallback;
   mReleaseCallback = releaseCallback;

   // Connect the intersection signal to our slot.
   mIsectConnections.push_back(
      mEventData->objectIntersected.connect(
         0, boost::bind(&MultiObjectGrabStrategy::objectIntersected, this,
                        _1, _2)
      )
   );

   // Connect the de-intersection signal to our slot.
   mIsectConnections.push_back(
      mEventData->objectDeintersected.connect(
         0, boost::bind(&MultiObjectGrabStrategy::objectDeintersected,
                        this, _1)
      )
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

void MultiObjectGrabStrategy::setFocus(ViewerPtr viewer, const bool focused)
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
         const bool has =
            status_panel_data->hasControlText(mChooseBtn.toString(),
                                              mChooseText);

         if ( ! has )
         {
            status_panel_data->addControlText(mChooseBtn.toString(),
                                              mChooseText, 1);
         }
      }

      // If grab button(s) is/are configured, we will update the status
      // panel to include that information.
      if ( mGrabBtn.isConfigured() )
      {
         const bool has =
            status_panel_data->hasControlText(mGrabBtn.toString(), mGrabText);

         if ( ! has )
         {
            status_panel_data->addControlText(mGrabBtn.toString(), mGrabText,
                                              1);
         }
      }

      // If release button(s) is/are configured, we will update the status
      // panel to include that information.
      if ( mReleaseBtn.isConfigured() )
      {
         const bool has =
            status_panel_data->hasControlText(mReleaseBtn.toString(),
                                              mReleaseText);

         if ( ! has )
         {
            status_panel_data->addControlText(mReleaseBtn.toString(),
                                              mReleaseText, 1);
         }
      }
   }
   else
   {
      ScenePtr scene = viewer->getSceneObj();
      StatusPanelDataPtr status_panel_data =
         scene->getSceneData<StatusPanelData>();

      status_panel_data->removeControlText(mChooseBtn.toString(),
                                           mChooseText);
      status_panel_data->removeControlText(mGrabBtn.toString(), mGrabText);
      status_panel_data->removeControlText(mReleaseBtn.toString(),
                                           mReleaseText);
   }
}

void MultiObjectGrabStrategy::update(ViewerPtr viewer)
{
   if ( ! mGrabbing )
   {
      // The user has requested to add mCurIsectObject to the collection of
      // objects selected for later grabbing.
      if ( mCurIsectObject && mCurIsectObject->isGrabbable() && mChooseBtn() )
      {
         std::vector<SceneObjectPtr>::iterator o =
            std::find(mChosenObjects.begin(), mChosenObjects.end(),
                      mCurIsectObject);

         // Only choose mCurIsectObject for grabbing if it is not already in
         // mChosenObjcts.
         if ( o == mChosenObjects.end() )
         {
            mChosenObjects.push_back(mCurIsectObject);

            // Connect the grabbable object state change signal to our slot.
            mObjConnections[mCurIsectObject] =
               mCurIsectObject->grabbableStateChanged().connect(
                  boost::bind(
                     &MultiObjectGrabStrategy::grabbableObjStateChanged,
                     this, _1
                  )
               );

            std::vector<SceneObjectPtr> objs(1, mCurIsectObject);
            mEventData->selectionListExpanded(objs);

            // Use the intersection point of the most recently chosen object.
            mIntersectPoint = mCurIntersectPoint;
         }
      }
      // The user has requested to grab all the selected objects (those in
      // mChosenObjects).
      else if ( mGrabBtn() )
      {
         // If mChosenObjects is not empty, those objects are the ones that
         // we will grab.
         if ( ! mChosenObjects.empty() )
         {
            mGrabbedObjects = mChosenObjects;
            mChosenObjects.clear();
            mGrabbing = true;
         }
         // If mChosenObjects is empty but we are intersecting an object,
         // then that will be the one that we grab.
         else if ( mCurIsectObject )
         {
            mGrabbedObjects.resize(1);
            mGrabbedObjects[0] = mCurIsectObject;
            mGrabbing = true;
         }

         // If mGrabbing is false at this point, then there is nothing to
         // grab. Otherwise, invoke mGrabCallback to indicate that one or more
         // objects are now grabbed.
         if ( mGrabbing )
         {
            mGrabCallback(mGrabbedObjects, mIntersectPoint);
         }
      }
   }
   // If we are grabbing an object and the release button has just been
   // pressed, then release the grabbed object.
   else if ( mGrabbing && mReleaseBtn() )
   {
      // We no longer care about grabbable state changes because
      // mGrabbedObjects is about to be emptied.
      std::for_each(
         mObjConnections.begin(), mObjConnections.end(),
         boost::bind(&boost::signals::connection::disconnect,
                     boost::bind(&obj_conn_map_t::value_type::second, _1))
      );

      // Update our state to reflect that we are no longer grabbing an object.
      mGrabbing = false;
      mReleaseCallback(mGrabbedObjects);
      mGrabbedObjects.clear();
   }
}

std::vector<SceneObjectPtr> MultiObjectGrabStrategy::getGrabbedObjects()
{
   return mGrabbedObjects;
}

void MultiObjectGrabStrategy::configure(jccl::ConfigElementPtr elt)
{
   vprASSERT(elt->getID() == getElementType());

   const unsigned int req_cfg_version(1);

   // Check for correct version of plugin configuration.
   if ( elt->getVersion() < req_cfg_version )
   {
      std::stringstream msg;
      msg << "Configuration of MultiObjectGrabStrategy failed.  Required "
          << "config element version is " << req_cfg_version
          << ", but element '" << elt->getName() << "' is version "
          << elt->getVersion();
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
}

event::ResultType MultiObjectGrabStrategy::
objectIntersected(SceneObjectPtr obj, const gmtl::Point3f& pnt)
{
   if ( ! mGrabbing )
   {
      // If we intersected a grabbable object.
      if ( obj->isGrabbable() )
      {
         mCurIsectObject = obj;
         mCurIntersectPoint = pnt;
      }
   }
   else
   {
      return event::DONE;
   }

   return event::CONTINUE;
}

event::ResultType MultiObjectGrabStrategy::
objectDeintersected(SceneObjectPtr obj)
{
   if ( mGrabbing )
   {
      return event::DONE;
   }

   mCurIsectObject = SceneObjectPtr();

   return event::CONTINUE;
}

void MultiObjectGrabStrategy::grabbableObjStateChanged(SceneObjectPtr obj)
{
   // If mChosenObjects is not empty, then we have a collection of objects to
   // grab that have not yet been grabbed. obj may be in mChosenObjects, and
   // if it is not grabbable, then it must be removed.
   if ( ! mChosenObjects.empty() )
   {
      std::vector<SceneObjectPtr>::iterator o =
         std::find(mChosenObjects.begin(), mChosenObjects.end(), obj);

      if ( o != mChosenObjects.end() && ! (*o)->isGrabbable() )
      {
         mObjConnections[*o].disconnect();
         mObjConnections.erase(*o);

         std::vector<SceneObjectPtr> objs(1, *o);
         mEventData->selectionListReduced(objs);

         mChosenObjects.erase(o);
      }
   }
   // If mGrabbedObjects is not empty, then we have grabbed objects, and obj
   // may be in that collection. If obj is no longer grabbable, it must be
   // removed from mGrabbedObjects.
   else if ( ! mGrabbedObjects.empty() )
   {
      std::vector<SceneObjectPtr>::iterator o =
         std::find(mGrabbedObjects.begin(), mGrabbedObjects.end(), obj);

      if ( o != mGrabbedObjects.end() && ! (*o)->isGrabbable() )
      {
         mObjConnections[*o].disconnect();
         mObjConnections.erase(*o);

         // Inform the code that is using us that we have released a grabbed
         // object.
         mReleaseCallback(std::vector<SceneObjectPtr>(1, *o));

         mGrabbedObjects.erase(o);

         // Keep the grabbing state up to date now that mGrabbedObjects has
         // changed.
         mGrabbing = ! mGrabbedObjects.empty();
      }
   }
}

}
