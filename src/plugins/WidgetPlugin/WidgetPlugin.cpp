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
#include <boost/assign/list_of.hpp>

#include <gmtl/Generate.h>
#include <gmtl/External/OpenSGConvert.h>

#include <vpr/vpr.h>
#include <jccl/Config/ConfigElement.h>

#include <vrkit/Status.h>
#include <vrkit/User.h>
#include <vrkit/Viewer.h>
#include <vrkit/Widget.h>
#include <vrkit/Version.h>
#include <vrkit/scenedata/EventData.h>
#include <vrkit/scenedata/StatusPanelData.h>
#include <vrkit/scenedata/WidgetData.h>
#include <vrkit/plugin/Creator.h>
#include <vrkit/plugin/Info.h>
#include <vrkit/exceptions/PluginException.h>

#include "WidgetPlugin.h"


using namespace boost::assign;

static const vrkit::plugin::Info sInfo(
   "com.infiscape", "WidgetPlugin",
   list_of(VRKIT_VERSION_MAJOR)(VRKIT_VERSION_MINOR)(VRKIT_VERSION_PATCH)
);
static vrkit::plugin::Creator<vrkit::viewer::Plugin> sPluginCreator(
   boost::bind(&vrkit::WidgetPlugin::create, sInfo)
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

WidgetPlugin::WidgetPlugin(const plugin::Info& info)
   : viewer::Plugin(info)
   , mSelectText("Activate/Deactivate Widgets")
//   , SelectedWidget(-1)
   , mWidgetPressed(false)
   , mSelectedObject()
{
   /* Do nothing. */ ;
}

viewer::PluginPtr WidgetPlugin::init(ViewerPtr viewer)
{
   mWandInterface = viewer->getUser()->getInterfaceTrader().getWandInterface();

   // XXX: Temporary hack.
   // Set up connection to block move event of grabbed object. This allows us
   // to select objects in the scene, but does not allow us to move them.
   ScenePtr scene = viewer->getSceneObj();
   EventDataPtr event_data = scene->getSceneData<EventData>();
   mMovedConnection =
      event_data->objectsMoved.connect(
         0, boost::bind(&WidgetPlugin::objectsMovedSlot, this, _1)
      );
   mMovedConnection.block();

   // Connect the intersection signal to our slot.
   mIsectConnection =
      event_data->objectIntersected.connect(
         0, boost::bind(&WidgetPlugin::objectIntersected, this, _1, _2)
      );

   // Connect the de-intersection signal to our slot.
   mDeIsectConnection =
      event_data->objectDeintersected.connect(
         0, boost::bind(&WidgetPlugin::objectDeintersected, this, _1)
      );

   event_data->objectsSelected.connect(
      0, boost::bind(&WidgetPlugin::objectsSelected, this, _1, true)
   );
   event_data->objectsDeselected.connect(
      0, boost::bind(&WidgetPlugin::objectsSelected, this, _1, false)
   );

   jccl::ConfigElementPtr cfg_elt =
      viewer->getConfiguration().getConfigElement(getElementType());

   if ( cfg_elt )
   {
      configure(cfg_elt);
   }
   else
   {
      // XXX: Default
      mSelectBtn.configure("0^", mWandInterface);
   }

   mWidgetData = scene->getSceneData<WidgetData>();

   return shared_from_this();
}

event::ResultType WidgetPlugin::objectIntersected(SceneObjectPtr obj,
                                                  const gmtl::Point3f& pnt)
{
   const std::vector<SceneObjectPtr>& objs = mWidgetData->getWidgets();

   if ( ! isFocused() )
   {
      return event::CONTINUE;
   }

   // Ensure that we intersected a widget and not a model etc.
   SceneObjectPtr parent = obj->getParent();
   while ( NULL != parent.get() )
   {
      if ( std::find(objs.begin(), objs.end(), parent) != objs.end() )
      {
         mIntersectedObj = obj;
         mIntersectPoint = pnt;
         //mIntersectSound.trigger();
         mIntersecting = true;
         mIntersectedObj->wandEntered();

         // Don't allow anyone else to process this event since it is only
         // for widgets. (ex. BasicHighlighter)
         return event::DONE;
      }
      parent = parent->getParent();
   }

   return event::CONTINUE;
}

event::ResultType WidgetPlugin::objectDeintersected(SceneObjectPtr obj)
{
   if ( mIntersectedObj != NULL &&
        mIntersectedObj == obj &&
        mIntersectedObj->getRoot() != OSG::NullFC )
   {
      if (isFocused())
      {
         mIntersectedObj->wandExited();
         mIntersecting = false;
         mIntersectedObj = SceneObjectPtr();
      }

      // Don't allow anyone else to process this event since it is only
      // for widgets. (ex. BasicHighlighter)
      return event::DONE;
   }

   return event::CONTINUE;
}

void WidgetPlugin::update(ViewerPtr viewer)
{
   if ( isFocused() )
   {
      // XXX: Mouse move is not currently implemented.
      //gmtl::Point3f last_isect_point = mIntersectPoint;

      // If we are intersecting an object but not grabbing it and the grab
      // button has just been pressed, grab the intersected object.
      if ( mIntersecting && ! mWidgetPressed && mSelectBtn() )
      {
         mWidgetPressed = true;

         mIntersectedObj->wandPressed();
      }
      // If we are grabbing an object and the grab button has just been
      // pressed again, release the grabbed object.
      else if ( mWidgetPressed && mSelectBtn() )
      {
         mWidgetPressed = false;

         // We have just released the grabbed object, but we are still
         // intersecting it.  Set the bounding box state back to the
         // intersecting state and clear mIntersectedObj.
         if ( mIntersectedObj != NULL &&
              mIntersectedObj->getRoot() != OSG::NullFC )
         {
            mIntersectedObj->wandReleased();
         }
      }

      // XXX: Mouse move is not currently implemented.
      /*
      if ( mIntersecting  &&  last_isect_point != mIntersectPoint)
      {
         static int i = 0;
         i++;

         mIntersectedObj->wandMoved();
      }
      */
   }

   if (mWidgetData->widgetsAddedOrRemoved())
   {
      OSG::GroupNodePtr decorator_root =
         viewer->getSceneObj()->getDecoratorRoot();

      WidgetData::widget_list_t widgets = mWidgetData->getWidgets();

#if OSG_MAJOR_VERSION < 2
      OSG::CPEditor dre(decorator_root.node(), OSG::Node::ChildrenFieldMask);
#endif

      // Add all widgets to decorator
      WidgetData::widget_list_t::iterator w;
      for ( w = widgets.begin(); w != widgets.end(); ++w )
      {
         if (decorator_root.node()->findChild((*w)->getRoot()))
         {
            decorator_root.node()->subChild((*w)->getRoot());
         }
         decorator_root.node()->addChild((*w)->getRoot());
         //(*g)->setVisible(mGridsVisible);
      }
   }
}

event::ResultType
WidgetPlugin::objectsSelected(const std::vector<SceneObjectPtr>& objs,
                              bool selected)
{
   if (selected)
   {
      // TODO: Extend for handling multiple selected objects?
      mSelectedObject = objs.front();
   }
   else
   {
      mSelectedObject = SceneObjectPtr();
   }
   return event::CONTINUE;
}

event::ResultType
WidgetPlugin::objectsMovedSlot(const EventData::moved_obj_list_t&)
{
   return event::DONE;
}

void WidgetPlugin::focusChanged(ViewerPtr viewer)
{
   ScenePtr scene = viewer->getSceneObj();
   StatusPanelDataPtr status_panel_data =
      scene->getSceneData<StatusPanelData>();

   if (isFocused())
   {
      /*
      mIsectConnection.unblock();
      mDeIsectConnection.unblock();
      mMovedConnection.unblock();
      */
   }
   else
   {
      /*
      mIsectConnection.block();
      mDeIsectConnection.block();
      mMovedConnection.block();
      */
   }
/*
   if ( ! isFocused() )
   {
      // If a grid is selected, deselect it. We allow grids to remain visible
      // when this plug-in is not focused, but there seems to be little value
      // to having a grid remain selected when this plug-in is not able to
      // receive user input to manipulate the selected grid.
      if ( mSelectedGridIndex != -1 )
      {
         mGrids[mSelectedGridIndex]->setSelected(false);
         mSelectedGridIndex = -1;
      }

      if ( status_panel_data->mStatusPanelPlugin )
      {
         StatusPanel& panel =
            status_panel_data->mStatusPanelPlugin->getPanel();

         if ( mActivateBtn.isConfigured() )
         {
            // The button numbers in mActivateBtn are zero-based, but we would
            // like them to be one-based in the status panel display.
            std::vector<int> btns(
               transformButtonVec(mActivateBtn.getButtons())
            );
            panel.removeControlText(btns, mActivateText);
         }

         if ( mCycleBtn.isConfigured() )
         {
            // The button numbers in mCycleBtn are zero-based, but we would
            // like them to be one-based in the status panel display.
            std::vector<int> btns(transformButtonVec(mCycleBtn.getButtons()));
            panel.removeControlText(btns, mCycleText);
         }

         if ( mHideBtn.isConfigured() )
         {
            // The button numbers in mHideBtn are zero-based, but we would
            // like them to be one-based in the status panel display.
            std::vector<int> btns(transformButtonVec(mHideBtn.getButtons()));
            panel.removeControlText(btns, mHideText);
         }

         if ( mResetBtn.isConfigured() )
         {
            // The button numbers in mResetBtn are zero-based, but we would
            // like them to be one-based in the status panel display.
            std::vector<int> btns(transformButtonVec(mResetBtn.getButtons()));
            panel.removeControlText(btns, mResetText);
         }
      }
   }
   else
   {
      if ( status_panel_data->mStatusPanelPlugin )
      {
         StatusPanel& panel =
            status_panel_data->mStatusPanelPlugin->getPanel();

         if ( mActivateBtn.isConfigured() )
         {
            // The button numbers in mActivateBtn are zero-based, but we would
            // like them to be one-based in the status panel display.
            std::vector<int> btns(
               transformButtonVec(mActivateBtn.getButtons())
            );

            if ( ! panel.hasControlText(btns, mActivateText) )
            {
               panel.addControlText(btns, mActivateText);
            }
         }

         if ( mCycleBtn.isConfigured() )
         {
            // The button numbers in mCycleBtn are zero-based, but we would
            // like them to be one-based in the status panel display.
            std::vector<int> btns(transformButtonVec(mCycleBtn.getButtons()));

            if ( ! panel.hasControlText(btns, mCycleText) )
            {
               panel.addControlText(btns, mCycleText);
            }
         }

         if ( mHideBtn.isConfigured() )
         {
            // The button numbers in mHideBtn are zero-based, but we would
            // like them to be one-based in the status panel display.
            std::vector<int> btns(transformButtonVec(mHideBtn.getButtons()));

            if ( ! panel.hasControlText(btns, mHideText) )
            {
               panel.addControlText(btns, mHideText);
            }
         }

         if ( mResetBtn.isConfigured() )
         {
            // The button numbers in mResetBtn are zero-based, but we would
            // like them to be one-based in the status panel display.
            std::vector<int> btns(transformButtonVec(mResetBtn.getButtons()));

            if ( ! panel.hasControlText(btns, mResetText) )
            {
               panel.addControlText(btns, mResetText);
            }
         }
      }
   }
   */
}

void WidgetPlugin::configure(jccl::ConfigElementPtr elt)
{
   vprASSERT(elt->getID() == getElementType());

   const unsigned int req_cfg_version(1);

   // Check for correct version of plugin configuration.
   if ( elt->getVersion() < req_cfg_version )
   {
      std::ostringstream msg;
      msg << "Configuration of WidgetPlugin failed.  Required config "
          << "element version is " << req_cfg_version << ", but element '"
          << elt->getName() << "' is version " << elt->getVersion();
      throw PluginException(msg.str(), VRKIT_LOCATION);
   }

   const std::string activate_btn_prop("activate_button_nums");
   const std::string activate1_btn_prop("activate1_button_nums");

   mSelectBtn.configure(elt->getProperty<std::string>(activate_btn_prop),
                        mWandInterface);
   mSelect1Btn.configure(elt->getProperty<std::string>(activate1_btn_prop),
                         mWandInterface);
}

}
