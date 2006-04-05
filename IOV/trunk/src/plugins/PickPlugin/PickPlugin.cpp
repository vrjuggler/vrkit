// Copyright (C) Infiscape Corporation 2005-2006

#include <string.h>
#include <algorithm>
#include <boost/bind.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <gmtl/Matrix.h>
#include <gmtl/MatrixOps.h>
#include <gmtl/External/OpenSGConvert.h>

#include <vpr/vpr.h>
#include <vpr/System.h>
#include <vpr/Util/FileUtils.h>

#include <IOV/EventData.h>
#include <IOV/GrabData.h>
#include <IOV/Viewer.h>
#include <IOV/PluginCreator.h>
#include <IOV/PluginPtr.h>
#include <IOV/User.h>
#include <IOV/InterfaceTrader.h>
#include <IOV/WandInterface.h>
#include <IOV/ViewPlatform.h>
#include <IOV/SceneObject.h>
#include <IOV/Status.h>
#include <IOV/StatusPanel.h>
#include <IOV/StatusPanelPlugin.h>
#include <IOV/Util/Exceptions.h>

#include "PickPlugin.h"


namespace fs = boost::filesystem;

static inf::PluginCreator<inf::Plugin> sPluginCreator(&inf::PickPlugin::create,
                                                      "Pick Plug-in");

extern "C"
{

/** @name Plug-in Entry Points */
//@{
IOV_PLUGIN_API(void) getPluginInterfaceVersion(vpr::Uint32& majorVer,
                                               vpr::Uint32& minorVer)
{
   majorVer = INF_PLUGIN_API_MAJOR;
   minorVer = INF_PLUGIN_API_MINOR;
}

IOV_PLUGIN_API(inf::PluginCreatorBase*) getCreator()
{
   return &sPluginCreator;
}
//@}

}

namespace inf
{

void PickPlugin::init(ViewerPtr viewer)
{
   mEventData = viewer->getSceneObj()->getSceneData<EventData>();
   mGrabData = viewer->getSceneObj()->getSceneData<GrabData>();

   // Connect the intersection signal to our slot.
   mIsectConnection = mEventData->mObjectIntersectedSignal.connect(0, boost::bind(&PickPlugin::objectIntersected, this, _1, _2, _3));

   // Connect the de-intersection signal to our slot.
   mDeIsectConnection = mEventData->mObjectDeintersectedSignal.connect(0, boost::bind(&PickPlugin::objectDeintersected, this, _1));

   InterfaceTrader& if_trader = viewer->getUser()->getInterfaceTrader();
   mWandInterface = if_trader.getWandInterface();

   mIntersectSound.init("intersect");
   mPickSound.init("pick");

   // Configure
   std::string elt_type_name = getElementType();
   jccl::ConfigElementPtr cfg_elt =
      viewer->getConfiguration().getConfigElement(elt_type_name);

   if ( cfg_elt )
   {
      // Configure it
      config(cfg_elt);
   }
}

inf::Event::ResultType
PickPlugin::objectIntersected(inf::SceneObjectPtr obj,
                              inf::SceneObjectPtr parentObj,
                              gmtl::Point3f pnt)
{
   const std::vector<SceneObjectPtr>& objs = mGrabData->getObjects();

   // If we intersected a grabbable object.
   if ( std::find(objs.begin(), objs.end(), parentObj) != objs.end() )
   {
      mIntersectedObj = obj;
      mIntersectPoint = pnt;
      mIntersectSound.trigger();
      mIntersecting = true;
   }
   
   return inf::Event::CONTINUE;
}

inf::Event::ResultType
PickPlugin::objectDeintersected(inf::SceneObjectPtr obj)
{
   mIntersecting = false;
   mIntersectedObj = SceneObjectPtr();

   return inf::Event::CONTINUE;
}

void PickPlugin::updateState(ViewerPtr viewer)
{
   // if Button pressed
   //   if mIsect == mPicked
   //     release mPicked
   //   else if mIsect != NULL
   //     release mPicked
   //     pick mIsect

   // If we are intersecting an object but not grabbing it and the grab
   // button has just been pressed, grab the intersected object.
   if ( mPickBtn.test(mWandInterface, gadget::Digital::TOGGLE_ON) &&
        NULL != mIntersectedObj)
   {
      if (mIntersectedObj == mPickedObj)
      {
         std::cout << "Double click deselect" << std::endl;
         mEventData->mObjectUnpickedSignal(mPickedObj);
         mPickedObj = SceneObjectPtr();
         mPicking = false;
      }
      else // We picked a new object.
      {
         if (mPickedObj != NULL)
         {
            std::cout << "New object selected, so deselect" << std::endl;
            mEventData->mObjectUnpickedSignal(mPickedObj);
            mPickedObj = SceneObjectPtr();
            mPicking = false;
         }

         mPickSound.trigger();
         mPickedObj = mIntersectedObj;
         mPicking = true;

         // Send a select event.
         mEventData->mObjectPickedSignal(mPickedObj);
      }
   }
}

void PickPlugin::run(inf::ViewerPtr viewer)
{
   ;
}

bool PickPlugin::config(jccl::ConfigElementPtr elt)
{
   vprASSERT(elt->getID() == getElementType());

   const unsigned int req_cfg_version(1);

   // Check for correct version of plugin configuration.
   if ( elt->getVersion() < req_cfg_version )
   {
      std::stringstream msg;
      msg << "Configuration of PickPlugin failed.  Required config "
          << "element version is " << req_cfg_version << ", but element '"
          << elt->getName() << "' is version " << elt->getVersion();
      throw PluginException(msg.str(), IOV_LOCATION);
   }

   const std::string pick_btn_prop("pick_button_nums");
   const std::string strategy_plugin_path_prop("strategy_plugin_path");
   const std::string isect_strategy_prop("isect_strategy");

   mPickBtn.configButtons(elt->getProperty<std::string>(pick_btn_prop));

   return true;
}

PickPlugin::PickPlugin()
   : mPickText("Select/Deselect Toggle")
   , mIntersecting(false)
   , mPicking(false)
{
   ;
}

struct IncValue
{
   int operator()(int v)
   {
      return v + 1;
   }
};

void PickPlugin::focusChanged(inf::ViewerPtr viewer)
{
   // If we have focus and our grab/release button is configured, we
   // will update the status panel to include our command.
   if ( isFocused() && mPickBtn.isConfigured() )
   {
      inf::ScenePtr scene = viewer->getSceneObj();
      StatusPanelPluginDataPtr status_panel_data =
         scene->getSceneData<StatusPanelPluginData>();

      if ( status_panel_data->mStatusPanelPlugin )
      {
         inf::StatusPanel& panel =
            status_panel_data->mStatusPanelPlugin->getPanel();
         if ( ! panel.hasControlText(mPickBtn.getButtons(), mPickText) )
         {
            // The button numbers in mPickBtn are zero-based, but we would like
            // them to be one-based in the status panel display.
            std::vector<int> btns(mPickBtn.getButtons().size());
            IncValue inc;
            std::transform(mPickBtn.getButtons().begin(),
                           mPickBtn.getButtons().end(), btns.begin(), inc);

            panel.addControlText(btns, mPickText);
         }
      }
   }
   else if ( ! isFocused() )
   {
      inf::ScenePtr scene = viewer->getSceneObj();
      StatusPanelPluginDataPtr status_panel_data =
         scene->getSceneData<StatusPanelPluginData>();

      if ( status_panel_data->mStatusPanelPlugin )
      {
         inf::StatusPanel& panel =
            status_panel_data->mStatusPanelPlugin->getPanel();

         // The button numbers in mPickBtn are zero-based, but we would like
         // them to be one-based in the status panel display.
         std::vector<int> btns(mPickBtn.getButtons().size());
         IncValue inc;
         std::transform(mPickBtn.getButtons().begin(),
                        mPickBtn.getButtons().end(), btns.begin(), inc);

         panel.removeControlText(btns, mPickText);
      }
   }
}

}
