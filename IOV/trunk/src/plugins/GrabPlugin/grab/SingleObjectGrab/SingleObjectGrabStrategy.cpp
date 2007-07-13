// Copyright (C) Infiscape Corporation 2005-2007

#include <IOV/Config.h>

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
#include <IOV/StatusPanelData.h>
#include <IOV/Version.h>
#include <IOV/Plugin/Info.h>
#include <IOV/Util/Exceptions.h>

#include "SingleObjectGrabStrategy.h"


static inf::PluginCreator<inf::GrabStrategy> sPluginCreator(
   &inf::SingleObjectGrabStrategy::create
);

extern "C"
{

/** @name Plug-in Entry Points */
//@{
IOV_PLUGIN_API(inf::plugin::Info) getPluginInfo()
{
   std::vector<unsigned int> version(3);
   version[0] = IOV_VERSION_MAJOR;
   version[1] = IOV_VERSION_MINOR;
   version[2] = IOV_VERSION_PATCH;

   return inf::plugin::Info("com.infiscape.grab", "SingleObjectGrabStrategy",
                            version);
}

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

SingleObjectGrabStrategy::SingleObjectGrabStrategy()
   : GrabStrategy()
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

GrabStrategyPtr SingleObjectGrabStrategy::
init(ViewerPtr viewer, grab_callback_t grabCallback,
     release_callback_t releaseCallback)
{
   mGrabCallback    = grabCallback;
   mReleaseCallback = releaseCallback;

   EventDataPtr event_data = viewer->getSceneObj()->getSceneData<EventData>();

   // Connect the intersection signal to our slot.
   mIsectConnection =
      event_data->mObjectIntersectedSignal.connect(
         0, boost::bind(&SingleObjectGrabStrategy::objectIntersected, this,
                        _1, _2)
      );

   // Connect the de-intersection signal to our slot.
   mDeIsectConnection =
      event_data->mObjectDeintersectedSignal.connect(
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
      inf::ScenePtr scene = viewer->getSceneObj();
      StatusPanelDataPtr status_panel_data =
         scene->getSceneData<StatusPanelData>();

      // If grab button(s) is/are configured, we will update the status
      // panel to include that information.
      if ( mGrabBtn.isConfigured() )
      {
         bool has = false;
         status_panel_data->mHasControlTexts(mGrabBtn.getButtons(), mGrabText, has);
         if ( ! has )
         {
            // The button numbers in mGrabBtn are zero-based, but we would
            // like them to be one-based in the status panel display.
            status_panel_data->mAddControlTexts(transformButtonVec(mGrabBtn.getButtons()),
                                 mGrabText, 1);
         }
      }
      // If release button(s) is/are configured, we will update the status
      // panel to include that information.
      if ( mReleaseBtn.isConfigured() )
      {
         bool has = false;
         status_panel_data->mHasControlTexts(mReleaseBtn.getButtons(), mReleaseText, has);
         if ( ! has )
         {
            // The button numbers in mReleaseBtn are zero-based, but we
            // would like them to be one-based in the status panel display.
            status_panel_data->mAddControlTexts(
               transformButtonVec(mReleaseBtn.getButtons()), mReleaseText, 1
            );
         }
      }
   }
   else
   {
      inf::ScenePtr scene = viewer->getSceneObj();
      StatusPanelDataPtr status_panel_data =
         scene->getSceneData<StatusPanelData>();

      // The button numbers in mGrabBtn are zero-based, but we would like
      // them to be one-based in the status panel display.
      status_panel_data->mRemoveControlTexts(transformButtonVec(mGrabBtn.getButtons()),
                              mGrabText);

      // The button numbers in mReleaseBtn are zero-based, but we would like
      // them to be one-based in the status panel display.
      status_panel_data->mRemoveControlTexts(transformButtonVec(mReleaseBtn.getButtons()),
                              mReleaseText);
   }
}

void SingleObjectGrabStrategy::update(ViewerPtr viewer)
{
   // If we are intersecting an object but not grabbing it, the grab
   // button has just been pressed, and the intersected object is grabbable,
   // then we can grab the intersected object.
   if ( mIntersecting && ! mGrabbing && mIntersectedObj->isGrabbable() &&
        mGrabBtn.test(mWandInterface, gadget::Digital::TOGGLE_ON) )
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
   else if ( mGrabbing &&
             mReleaseBtn.test(mWandInterface, gadget::Digital::TOGGLE_ON) )
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
      throw PluginException(msg.str(), IOV_LOCATION);
   }

   const std::string grab_btn_prop("grab_button_nums");
   const std::string release_btn_prop("release_button_nums");

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

std::vector<int> SingleObjectGrabStrategy::
transformButtonVec(const std::vector<int>& btns)
{
   std::vector<int> result(btns.size());
   IncValue inc;
   std::transform(btns.begin(), btns.end(), result.begin(), inc);
   return result;
}

inf::Event::ResultType SingleObjectGrabStrategy::
objectIntersected(SceneObjectPtr obj, const gmtl::Point3f& pnt)
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
      return inf::Event::DONE;
   }
   
   return inf::Event::CONTINUE;
}

inf::Event::ResultType SingleObjectGrabStrategy::
objectDeintersected(SceneObjectPtr obj)
{
   if ( mGrabbing )
   {
      return inf::Event::DONE;
   }

   mIntersecting = false;
   mIntersectedObj = SceneObjectPtr();

   return inf::Event::CONTINUE;
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

void SingleObjectGrabStrategy::
grabbableObjStateChanged(inf::SceneObjectPtr obj)
{
   // If we are currently grabbing obj and its grabbable state has changed so
   // that it is no longer grabbable, we must release it.
   if ( mGrabbing && mGrabbedObj == obj && ! mGrabbedObj->isGrabbable() )
   {
      releaseGrabbedObject();
   }
}

}
