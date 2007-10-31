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

#include <boost/bind.hpp>
#include <boost/assign/list_of.hpp>

#include <vrkit/Scene.h>
#include <vrkit/Viewer.h>
#include <vrkit/User.h>
#include <vrkit/InterfaceTrader.h>
#include <vrkit/WandInterface.h>
#include <vrkit/SceneObject.h>
#include <vrkit/Version.h>
#include <vrkit/scenedata/EventData.h>
#include <vrkit/scenedata/StatusPanelData.h>
#include <vrkit/plugin/Creator.h>
#include <vrkit/plugin/Info.h>
#include <vrkit/exceptions/PluginException.h>

#include "PickPlugin.h"


using namespace boost::assign;

static const vrkit::plugin::Info sInfo(
   "com.infiscape", "PickPlugin",
   list_of(VRKIT_VERSION_MAJOR)(VRKIT_VERSION_MINOR)(VRKIT_VERSION_PATCH)
);
static vrkit::plugin::Creator<vrkit::viewer::Plugin> sPluginCreator(
   boost::bind(&vrkit::PickPlugin::create, sInfo)
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

PickPlugin::PickPlugin(const plugin::Info& info)
   : viewer::Plugin(info)
   , mPickText("Select/Deselect Toggle")
   , mIntersecting(false)
   , mPicking(false)
{
   ;
}

viewer::PluginPtr PickPlugin::init(ViewerPtr viewer)
{
   mEventData = viewer->getSceneObj()->getSceneData<EventData>();

   // Connect the intersection signal to our slot.
   mIsectConnection =
      mEventData->objectIntersected.connect(
         0, boost::bind(&PickPlugin::objectIntersected, this, _1, _2)
      );

   // Connect the de-intersection signal to our slot.
   mDeIsectConnection =
      mEventData->objectDeintersected.connect(
         0, boost::bind(&PickPlugin::objectDeintersected, this, _1)
      );

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

   return shared_from_this();
}

event::ResultType PickPlugin::objectIntersected(SceneObjectPtr obj,
                                                gmtl::Point3f pnt)
{
   // If we intersected a grabbable object.
   if ( obj->isGrabbable() )
   {
      mIntersectedObj = obj;
      mIntersectPoint = pnt;
      mIntersectSound.trigger();
      mIntersecting = true;
   }

   return event::CONTINUE;
}

event::ResultType PickPlugin::objectDeintersected(SceneObjectPtr obj)
{
   mIntersecting = false;
   mIntersectedObj = SceneObjectPtr();

   return event::CONTINUE;
}

void PickPlugin::update(ViewerPtr)
{
   if ( isFocused() )
   {
      // if Button pressed
      //   if mIsect == mPicked
      //     release mPicked
      //   else if mIsect != NULL
      //     release mPicked
      //     pick mIsect

      // If we are intersecting an object but not grabbing it and the grab
      // button has just been pressed, grab the intersected object.
      if ( mPickBtn() && NULL != mIntersectedObj.get() )
      {
         if (mIntersectedObj == mPickedObj)
         {
            std::cout << "Double click deselect" << std::endl;
            mEventData->objectUnpicked(mPickedObj);
            mPickedObj = SceneObjectPtr();
            mPicking = false;
         }
         else // We picked a new object.
         {
            if (mPickedObj != NULL)
            {
               std::cout << "New object selected, so deselect" << std::endl;
               mEventData->objectUnpicked(mPickedObj);
               mPickedObj = SceneObjectPtr();
               mPicking = false;
            }

            mPickSound.trigger();
            mPickedObj = mIntersectedObj;
            mPicking = true;

            // Send a select event.
            mEventData->objectPicked(mPickedObj);
         }
      }
   }
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
      throw PluginException(msg.str(), VRKIT_LOCATION);
   }

   const std::string pick_btn_prop("pick_button_nums");
   const std::string strategy_plugin_path_prop("strategy_plugin_path");
   const std::string isect_strategy_prop("isect_strategy");

   mPickBtn.configure(elt->getProperty<std::string>(pick_btn_prop),
                      mWandInterface);

   return true;
}

void PickPlugin::focusChanged(ViewerPtr viewer)
{
   // If we have focus and our grab/release button is configured, we
   // will update the status panel to include our command.
   if ( isFocused() && mPickBtn.isConfigured() )
   {
      ScenePtr scene = viewer->getSceneObj();
      StatusPanelDataPtr status_panel_data =
         scene->getSceneData<StatusPanelData>();
      status_panel_data->addControlText(mPickBtn.toString(), mPickText, 1);
   }
   else if ( ! isFocused() )
   {
      ScenePtr scene = viewer->getSceneObj();
      StatusPanelDataPtr status_panel_data =
         scene->getSceneData<StatusPanelData>();
      status_panel_data->removeControlText(mPickBtn.toString(), mPickText);
   }
}

}
