// Copyright (C) Infiscape Corporation 2005-2006

#include <algorithm>

#include <gmtl/Generate.h>
#include <gmtl/External/OpenSGConvert.h>

#include <vpr/vpr.h>
#include <jccl/Config/ConfigElement.h>

#include <IOV/EventData.h>
#include <IOV/PluginCreator.h>
#include <IOV/StatusPanel.h>
#include <IOV/StatusPanelPlugin.h>
#include <IOV/StatusPanelData.h>
#include <IOV/Status.h>
#include <IOV/User.h>
#include <IOV/Viewer.h>
#include <IOV/Widget/Widget.h>
#include <IOV/Widget/WidgetData.h>

#include <boost/bind.hpp>

#include "WidgetPlugin.h"


static inf::PluginCreator<inf::Plugin> sPluginCreator(&inf::WidgetPlugin::create,
                                                      "Widget Plug-in");

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

WidgetPlugin::WidgetPlugin()
   : mSelectText("Activate/Deactivate Widgets")
//   , SelectedWidget(-1)
   , mWidgetPressed(false)
   , mSelectedObject()
{
   /* Do nothing. */ ;
}

PluginPtr WidgetPlugin::init(inf::ViewerPtr viewer)
{
   mWandInterface = viewer->getUser()->getInterfaceTrader().getWandInterface();

   // XXX: Temporary hack.
   // Set up connection to block move event of grabbed object. This allows us
   // to select objects in the scene, but does not allow us to move them.
   inf::ScenePtr scene = viewer->getSceneObj();
   EventDataPtr event_data = scene->getSceneData<inf::EventData>();
   mMovedConnection =
      event_data->mObjectsMovedSignal.connect(
         0, boost::bind(&WidgetPlugin::objectsMovedSlot, this, _1)
      );
   mMovedConnection.block();

   // Connect the intersection signal to our slot.
   mIsectConnection = event_data->mObjectIntersectedSignal.connect(0, boost::bind(&WidgetPlugin::objectIntersected, this, _1, _2));

   // Connect the de-intersection signal to our slot.
   mDeIsectConnection = event_data->mObjectDeintersectedSignal.connect(0, boost::bind(&WidgetPlugin::objectDeintersected, this, _1));

   event_data->mObjectsSelectedSignal.connect(
      0, boost::bind(&WidgetPlugin::objectsSelected, this, _1, true)
   );
   event_data->mObjectsDeselectedSignal.connect(
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
      mSelectBtn.configButtons("0");
   }

   mWidgetData = scene->getSceneData<WidgetData>();

   return shared_from_this();
}

inf::Event::ResultType
WidgetPlugin::objectIntersected(inf::SceneObjectPtr obj,
                                const gmtl::Point3f& pnt)
{
   const std::vector<SceneObjectPtr>& objs = mWidgetData->getWidgets();

   if (!isFocused())
   {
      return inf::Event::CONTINUE;
   }

   // Ensure that we intersected a widget and not a model etc.
   inf::SceneObjectPtr parent = obj->getParent();
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
         return inf::Event::DONE;
      }
      parent = parent->getParent();
   }

   return inf::Event::CONTINUE;
}

inf::Event::ResultType
WidgetPlugin::objectDeintersected(inf::SceneObjectPtr obj)
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
      return inf::Event::DONE;
   }

   return inf::Event::CONTINUE;
}

void WidgetPlugin::update(inf::ViewerPtr viewer)
{
   if ( isFocused() )
   {
      // XXX: Mouse move is not currently implemented.
      //gmtl::Point3f last_isect_point = mIntersectPoint;

      // If we are intersecting an object but not grabbing it and the grab
      // button has just been pressed, grab the intersected object.
      if ( mIntersecting && ! mWidgetPressed &&
           mSelectBtn.test(mWandInterface, gadget::Digital::TOGGLE_ON) )
      {
         mWidgetPressed = true;

         mIntersectedObj->wandPressed();
      }
      // If we are grabbing an object and the grab button has just been
      // pressed again, release the grabbed object.
      else if ( mWidgetPressed &&
                mSelectBtn.test(mWandInterface, gadget::Digital::TOGGLE_OFF) )
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

      // Add all widgets to decorator
      WidgetData::widget_list_t::iterator w;
      for ( w = widgets.begin(); w != widgets.end(); ++w )
      {
         OSG::beginEditCP(decorator_root.node(), OSG::Node::ChildrenFieldMask);
            if (decorator_root.node()->findChild((*w)->getRoot()))
            {
               decorator_root.node()->subChild((*w)->getRoot());
            }
            decorator_root.node()->addChild((*w)->getRoot());
         OSG::endEditCP(decorator_root.node(), OSG::Node::ChildrenFieldMask);
         //(*g)->setVisible(mGridsVisible);
      }
   }
}

inf::Event::ResultType WidgetPlugin::
objectsSelected(const std::vector<inf::SceneObjectPtr>& objs, bool selected)
{
   if (selected)
   {
      // TODO: Extend for handling multiple selected objects?
      mSelectedObject = objs.front();
   }
   else
   {
      mSelectedObject = inf::SceneObjectPtr();
   }
   return inf::Event::CONTINUE;
}

inf::Event::ResultType WidgetPlugin::
objectsMovedSlot(const EventData::moved_obj_list_t&)
{
   return inf::Event::DONE;
}

void WidgetPlugin::focusChanged(inf::ViewerPtr viewer)
{
   inf::ScenePtr scene = viewer->getSceneObj();
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
         inf::StatusPanel& panel =
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
         inf::StatusPanel& panel =
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
      throw PluginException(msg.str(), IOV_LOCATION);
   }

   const std::string activate_btn_prop("activate_button_nums");
   const std::string activate1_btn_prop("activate1_button_nums");

   mSelectBtn.configButtons(elt->getProperty<std::string>(activate_btn_prop));
   mSelect1Btn.configButtons(elt->getProperty<std::string>(activate1_btn_prop));
}

struct IncValue
{
   int operator()(int v)
   {
      return v + 1;
   }
};

std::vector<int> WidgetPlugin::transformButtonVec(const std::vector<int>& btns)
{
   std::vector<int> result(btns.size());
   IncValue inc;
   std::transform(btns.begin(), btns.end(), result.begin(), inc);
   return result;
}

}
