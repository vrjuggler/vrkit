// Copyright (C) Infiscape Corporation 2005-2006

#include <IOV/Config.h>

#include <functional>
#include <boost/bind.hpp>

#include <gmtl/External/OpenSGConvert.h>

#include <vpr/Util/Assert.h>
#include <jccl/Config/ConfigElement.h>

#include <IOV/Plugin/PluginConfig.h>
#include <IOV/PluginCreator.h>
#include <IOV/EventData.h>
#include <IOV/Scene.h>
#include <IOV/User.h>
#include <IOV/Viewer.h>
#include <IOV/InterfaceTrader.h>
#include <IOV/WandInterface.h>
#include <IOV/SceneObject.h>
#include <IOV/StatusPanel.h>
#include <IOV/StatusPanelPlugin.h>

#include "MultiObjectGrabStrategy.h"


static inf::PluginCreator<inf::GrabStrategy> sPluginCreator(
   &inf::MultiObjectGrabStrategy::create, "Multi-Object Grab Strategy Plug-in"
);

extern "C"
{

/** @name Plug-in Entry Points */
//@{
IOV_PLUGIN_API(void) getPluginInterfaceVersion(vpr::Uint32& majorVer,
                                               vpr::Uint32& minorVer)
{
   majorVer = INF_GRAB_STRATEGY_PLUGIN_API_MAJOR;
   minorVer = INF_GRAB_STRATEGY_PLUGIN_API_MINOR;
}

IOV_PLUGIN_API(inf::PluginCreatorBase*) getGrabStrategyCreator()
{
   return &sPluginCreator;
}
//@}

}

namespace inf
{

MultiObjectGrabStrategy::MultiObjectGrabStrategy()
   : GrabStrategy()
   , mChooseText("Choose object to grab")
   , mGrabText("Grab object(s)")
   , mReleaseText("Release object(s)")
   , mGrabbing(false)
{
   /* Do nothing. */ ;
}

MultiObjectGrabStrategy::~MultiObjectGrabStrategy()
{
   std::for_each(mIsectConnections.begin(), mIsectConnections.end(),
                 boost::bind(&boost::signals::connection::disconnect, _1));
}

GrabStrategyPtr MultiObjectGrabStrategy::init(ViewerPtr viewer)
{
   mEventData = viewer->getSceneObj()->getSceneData<EventData>();

   // Connect the intersection signal to our slot.
   mIsectConnections.push_back(
      mEventData->mObjectIntersectedSignal.connect(
         0, boost::bind(&MultiObjectGrabStrategy::objectIntersected, this,
                        _1, _2, _3, viewer)
      )
   );

   // Connect the de-intersection signal to our slot.
   mIsectConnections.push_back(
      mEventData->mObjectDeintersectedSignal.connect(
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
      inf::ScenePtr scene = viewer->getSceneObj();
      StatusPanelPluginDataPtr status_panel_data =
         scene->getSceneData<StatusPanelPluginData>();

      if ( status_panel_data->mStatusPanelPlugin )
      {
         // If choose button(s) is/are configured, we will update the status
         // panel to include that information.
         if ( mChooseBtn.isConfigured() )
         {
            inf::StatusPanel& panel =
               status_panel_data->mStatusPanelPlugin->getPanel();
            if ( ! panel.hasControlText(mChooseBtn.getButtons(), mChooseText) )
            {
               // The button numbers in mChooseBtn are zero-based, but we would
               // like them to be one-based in the status panel display.
               panel.addControlText(
                  transformButtonVec(mChooseBtn.getButtons()), mChooseText
               );
            }
         }

         // If grab button(s) is/are configured, we will update the status
         // panel to include that information.
         if ( mGrabBtn.isConfigured() )
         {
            inf::StatusPanel& panel =
               status_panel_data->mStatusPanelPlugin->getPanel();
            if ( ! panel.hasControlText(mGrabBtn.getButtons(), mGrabText) )
            {
               // The button numbers in mGrabBtn are zero-based, but we would
               // like them to be one-based in the status panel display.
               panel.addControlText(transformButtonVec(mGrabBtn.getButtons()),
                                    mGrabText);
            }
         }

         // If release button(s) is/are configured, we will update the status
         // panel to include that information.
         if ( mReleaseBtn.isConfigured() )
         {
            inf::StatusPanel& panel =
               status_panel_data->mStatusPanelPlugin->getPanel();
            if ( ! panel.hasControlText(mReleaseBtn.getButtons(), mReleaseText) )
            {
               // The button numbers in mReleaseBtn are zero-based, but we
               // would like them to be one-based in the status panel display.
               panel.addControlText(
                  transformButtonVec(mReleaseBtn.getButtons()), mReleaseText
               );
            }
         }
      }
   }
   else
   {
      inf::ScenePtr scene = viewer->getSceneObj();
      StatusPanelPluginDataPtr status_panel_data =
         scene->getSceneData<StatusPanelPluginData>();

      if ( status_panel_data->mStatusPanelPlugin )
      {
         inf::StatusPanel& panel =
            status_panel_data->mStatusPanelPlugin->getPanel();

         // The button numbers in mChooseBtn are zero-based, but we would like
         // them to be one-based in the status panel display.
         panel.removeControlText(transformButtonVec(mChooseBtn.getButtons()),
                                 mChooseText);

         // The button numbers in mGrabBtn are zero-based, but we would like
         // them to be one-based in the status panel display.
         panel.removeControlText(transformButtonVec(mGrabBtn.getButtons()),
                                 mGrabText);

         // The button numbers in mReleaseBtn are zero-based, but we would like
         // them to be one-based in the status panel display.
         panel.removeControlText(transformButtonVec(mReleaseBtn.getButtons()),
                                 mReleaseText);
      }
   }
}

void MultiObjectGrabStrategy::update(ViewerPtr viewer,
                                     grab_callback_t grabCallback,
                                     release_callback_t releaseCallback)
{
   if ( ! mGrabbing )
   {
      // The user has requested to add mCurIsectObject to the collection of
      // objects selected for later grabbing.
      if ( mCurIsectObject && mCurIsectObject->isGrabbable() &&
           mChooseBtn.test(mWandInterface, gadget::Digital::TOGGLE_ON) )
      {
         mChosenObjects.push_back(mCurIsectObject);

         // Connect the grabbable object state change signal to our slot.
         mObjConnections[mCurIsectObject] =
            mCurIsectObject->grabbableStateChanged().connect(
               boost::bind(&MultiObjectGrabStrategy::grabbableObjStateChanged,
                           this, _1)
            );

         // Use the intersection point of the most recently chosen object.
         mIntersectPoint = mCurIntersectPoint;
      }
      // The user has requested to grab all the selected objects (those in
      // mChosenObjects).
      else if ( mGrabBtn.test(mWandInterface, gadget::Digital::TOGGLE_ON) )
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
         // grab. Otherwise, invoke grabCallback to indicate that one or more
         // objects are now grabbed.
         if ( mGrabbing )
         {
            grabCallback(mGrabbedObjects, mIntersectPoint);
         }
      }
   }
   // If we are grabbing an object and the release button has just been
   // pressed, then release the grabbed object.
   else if ( mGrabbing &&
             mReleaseBtn.test(mWandInterface, gadget::Digital::TOGGLE_ON) )
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
      releaseCallback(mGrabbedObjects);
      mGrabbedObjects.clear();
   }
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
      throw PluginException(msg.str(), IOV_LOCATION);
   }

   const std::string choose_btn_prop("choose_button_nums");
   const std::string grab_btn_prop("grab_button_nums");
   const std::string release_btn_prop("release_button_nums");

   mChooseBtn.configButtons(elt->getProperty<std::string>(choose_btn_prop));
   mGrabBtn.configButtons(elt->getProperty<std::string>(grab_btn_prop));
   mReleaseBtn.configButtons(elt->getProperty<std::string>(release_btn_prop));
}

struct IncValue
{
   int operator()(int v)
   {
      return v + 1;
   }
};

std::vector<int> MultiObjectGrabStrategy::
transformButtonVec(const std::vector<int>& btns)
{
   std::vector<int> result(btns.size());
   IncValue inc;
   std::transform(btns.begin(), btns.end(), result.begin(), inc);
   return result;
}

inf::Event::ResultType MultiObjectGrabStrategy::
objectIntersected(SceneObjectPtr obj, SceneObjectPtr parentObj,
                  gmtl::Point3f pnt, ViewerPtr viewer)
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
      return inf::Event::DONE;
   }
   
   return inf::Event::CONTINUE;
}

inf::Event::ResultType MultiObjectGrabStrategy::
objectDeintersected(SceneObjectPtr obj)
{
   if ( mGrabbing )
   {
      return inf::Event::DONE;
   }

   mCurIsectObject = SceneObjectPtr();

   return inf::Event::CONTINUE;
}

void MultiObjectGrabStrategy::grabbableObjStateChanged(inf::SceneObjectPtr obj)
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
         mChosenObjects.erase(o);
      }
   }
   // If mGrabbedObjects is not empty, then have grabbed objects, and obj may
   // be in that collection. If obj is no longer grabbable, it must be
   // removed from mGrabbedObjects.
   else if ( ! mGrabbedObjects.empty() )
   {
      std::vector<SceneObjectPtr>::iterator o =
         std::find(mChosenObjects.begin(), mChosenObjects.end(), obj);

      if ( o != mChosenObjects.end() && ! (*o)->isGrabbable() )
      {
         mObjConnections[*o].disconnect();
         mObjConnections.erase(*o);
         mGrabbedObjects.erase(o);
      }
   }
}

}
