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

void WidgetPlugin::init(inf::ViewerPtr viewer)
{
   mWandInterface = viewer->getUser()->getInterfaceTrader().getWandInterface();

   // XXX: Temporary hack.
   // Set up connection to block move event of grabbed object. This allows us
   // to select objects in the scene, but does not allow us to move them.
   inf::ScenePtr scene = viewer->getSceneObj();
   EventDataPtr event_data = scene->getSceneData<inf::EventData>();
   mMovedConnection = event_data->mObjectMovedSignal.connect(0, boost::bind(&WidgetPlugin::objectMovedSlot, this, _1, _2));
   mMovedConnection.block();

   event_data->mObjectSelectedSignal.connect(0, boost::bind(&WidgetPlugin::objectSelected, this, _1, true));
   event_data->mObjectDeselectedSignal.connect(0, boost::bind(&WidgetPlugin::objectSelected, this, _1, false));

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


   // Initialize the plug-in factories for our strategy plug-in types.
   mPluginFactory = viewer->getPluginFactory();
   //mPluginFactory->addScanPath(mStrategyPluginPath);

   mIsectStrategyName = "RayIntersection";
   // Build the IntersectionStrategy instance.
   try
   {
      IOV_STATUS << "   Loading intersection strategy plug-in '"
                 << mIsectStrategyName << "' ..." << std::flush;
      inf::PluginCreator<inf::IntersectionStrategy>* creator =
         mPluginFactory->getPluginCreator<inf::IntersectionStrategy>(mIsectStrategyName);

      if ( NULL != creator )
      {
         mIsectStrategy = creator->createPlugin();
         mIsectStrategy->init(viewer);
         IOV_STATUS << "[OK]" << std::endl;
      }
      else
      {
         IOV_STATUS << "[ERROR]\nWARNING: No creator for strategy plug-in "
                    << mIsectStrategyName << std::endl;
      }
   }
   catch (std::runtime_error& ex)
   {
      IOV_STATUS << "[ERROR]\nWARNING: Failed to load strategy plug-in "
                 << mIsectStrategyName << ":\n" << ex.what() << std::endl;
   }
}

void WidgetPlugin::updateState(inf::ViewerPtr viewer)
{
   gmtl::Point3f last_isect_point = mIntersectPoint;

   // Get the intersected object.
   SceneObjectPtr intersect_obj;
   if (NULL != mIsectStrategy.get())
   {
      gmtl::Point3f cur_ip;
      std::vector<SceneObjectPtr> objs = mWidgetData->getWidgets();
      SceneObjectPtr cur_obj = mIsectStrategy->findIntersection(viewer, objs, cur_ip);

      // Save results from calling intersect on grabbable objects.
      intersect_obj = cur_obj;
      mIntersectPoint = cur_ip;

      while (NULL != cur_obj && intersect_obj->hasChildren())
      {
         objs = cur_obj->getChildren();
         cur_ip.set(0.0f, 0.0f, 0.0f);
         cur_obj = mIsectStrategy->findIntersection(viewer, objs, cur_ip);

         // If we intersected a child, save results
         if (NULL != cur_obj)
         {
            intersect_obj = cur_obj;
            mIntersectPoint = cur_ip;
         }
      }
   }

   // If the intersected object is different than the one with which the
   // wand intersected during the last frame, we need to make updates to
   // the application and scene state.
   if ( intersect_obj != mIntersectedObj )
   {
      if ( mIntersectedObj != NULL && mIntersectedObj->getRoot() != OSG::NullFC )
      {
         mIntersectedObj->wandExited();
      }

      // Change the intersected object to the one we found above.
      mIntersectedObj = intersect_obj;

      // If the new node of mIntersectedObj is non-NULL, then we are
      // intersecting a new object since the last frame.  Set up the
      // highlight node for this new object.
      if ( mIntersectedObj != NULL && mIntersectedObj->getRoot() != OSG::NullFC)
      {
         //mIntersectSound.trigger();
         mIntersecting = true;

         mIntersectedObj->wandEntered();
      }
      // Otherwise, we are intersecting nothing.
      else
      {
         mIntersecting = false;
      }
   }

   // If we are intersecting an object but not grabbing it and the grab
   // button has just been pressed, grab the intersected object.
   if ( mIntersecting && !mWidgetPressed &&
        mSelectBtn.test(mWandInterface, gadget::Digital::TOGGLE_ON) )
   {
      mWidgetPressed = true;

      mIntersectedObj->wandPressed();
   }
   // If we are grabbing an object and the grab button has just been pressed
   // again, release the grabbed object.
   else if ( mWidgetPressed &&
             mSelectBtn.test(mWandInterface, gadget::Digital::TOGGLE_OFF) )
   {
      mWidgetPressed = false;

      // We have just released the grabbed object, but we are still
      // intersecting it.  Set the bounding box state back to the
      // intersecting state and clear mIntersectedObj.
      if ( mIntersectedObj != NULL && mIntersectedObj->getRoot() != OSG::NullFC )
      {
         mIntersectedObj->wandReleased();
      }
   }

   // Move the grabbed object.
   if ( mIntersecting  &&  last_isect_point != mIntersectPoint)
   {
      // Get the wand transformation in virtual world coordinates.
      /*
      const gmtl::Matrix44f vp_M_wand_xform(
         mWandInterface->getWandPos()->getData(viewer->getDrawScaleFactor())
      );
      */
      static int i = 0;
      i++;

      mIntersectedObj->wandMoved();
   }
}

void WidgetPlugin::run(inf::ViewerPtr viewer)
{
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

WidgetPlugin::WidgetPlugin()
   : mSelectText("Activate/Deactivate Widgets")
     //,SelectedWidget(-1)
     ,mWidgetPressed(false)
     ,mSelectedObject()
{
   /* Do nothing. */ ;
}

inf::Event::ResultType WidgetPlugin::objectSelected(inf::SceneObjectPtr obj, bool selected)
{
   if (selected)
   {
      mSelectedObject = obj;
   }
   else
   {
      mSelectedObject = inf::SceneObjectPtr();
   }
   return inf::Event::CONTINUE;
}

inf::Event::ResultType WidgetPlugin::objectMovedSlot(inf::SceneObjectPtr obj, const gmtl::Matrix44f& newObjMat)
{
   return inf::Event::DONE;
}

void WidgetPlugin::focusChanged(inf::ViewerPtr viewer)
{
   inf::ScenePtr scene = viewer->getSceneObj();
   StatusPanelPluginDataPtr status_panel_data =
      scene->getSceneData<StatusPanelPluginData>();

   if (isFocused())
   {
      mMovedConnection.unblock();
   }
   else
   {
      mMovedConnection.block();
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
   throw (inf::Exception)
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
