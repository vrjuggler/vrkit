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

#include "GrabPlugin.h"


namespace fs = boost::filesystem;

static inf::PluginCreator<inf::Plugin> sPluginCreator(&inf::GrabPlugin::create,
                                                      "Grab Plug-in");

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

PluginPtr GrabPlugin::init(ViewerPtr viewer)
{
   mEventData = viewer->getSceneObj()->getSceneData<EventData>();
   mEventData->mObjectMovedSignal.connect(100, boost::bind(&GrabPlugin::defaultObjectMovedSlot, this, _1, _2));

   // Connect the intersection signal to our slot.
   mIsectConnection = mEventData->mObjectIntersectedSignal.connect(0, boost::bind(&GrabPlugin::objectIntersected, this, _1, _2, _3));

   // Connect the de-intersection signal to our slot.
   mDeIsectConnection = mEventData->mObjectDeintersectedSignal.connect(0, boost::bind(&GrabPlugin::objectDeintersected, this, _1));

   mGrabData = viewer->getSceneObj()->getSceneData<GrabData>();

   InterfaceTrader& if_trader = viewer->getUser()->getInterfaceTrader();
   mWandInterface = if_trader.getWandInterface();

   mIntersectSound.init("intersect");
   mGrabSound.init("grab");

   // Configure
   std::string elt_type_name = getElementType();
   jccl::ConfigElementPtr cfg_elt =
      viewer->getConfiguration().getConfigElement(elt_type_name);

   if ( cfg_elt )
   {
      // Configure it
      config(cfg_elt);
   }

   // Initialize the plug-in factories for our strategy plug-in types.
   mPluginFactory = viewer->getPluginFactory();
   mPluginFactory->addScanPath(mStrategyPluginPath);

   // Build the MoveStrategies
   for (std::vector<std::string>::iterator itr = mMoveStrategyNames.begin();
        itr != mMoveStrategyNames.end(); ++itr)
   {
      try
      {
         IOV_STATUS << "   Loading move strategy plug-in '"
                    << *itr << "' ..." << std::flush;
         inf::PluginCreator<inf::MoveStrategy>* creator =
            mPluginFactory->getPluginCreator<inf::MoveStrategy>(*itr);

         if ( NULL != creator )
         {
            MoveStrategyPtr move_strategy = creator->createPlugin();
            move_strategy->init(viewer);
            mMoveStrategies.push_back(move_strategy);
            IOV_STATUS << "[OK]" << std::endl;
         }
         else
         {
            IOV_STATUS << "[ERROR]\nWARNING: No creator for strategy plug-in "
                       << *itr << std::endl;
         }
      }
      catch (std::runtime_error& ex)
      {
         IOV_STATUS << "[ERROR]\nWARNING: Failed to load strategy plug-in "
                    << *itr << ":\n" << ex.what() << std::endl;
      }
   }

   return shared_from_this();
}

inf::Event::ResultType
GrabPlugin::objectIntersected(inf::SceneObjectPtr obj,
                              inf::SceneObjectPtr parentObj,
                              gmtl::Point3f pnt)
{
   if (!mGrabbing)
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
   }
   else
   {
      return inf::Event::DONE;
   }
   
   return inf::Event::CONTINUE;
}

inf::Event::ResultType
GrabPlugin::objectDeintersected(inf::SceneObjectPtr obj)
{
   if (mGrabbing)
   {
      return inf::Event::DONE;
   }

   mIntersecting = false;
   mIntersectedObj = SceneObjectPtr();

   return inf::Event::CONTINUE;
}


void GrabPlugin::updateState(ViewerPtr viewer)
{
   // Get the wand transformation in virtual platform coordinates.
   const gmtl::Matrix44f vp_M_wand_xform(
      mWandInterface->getWandPos()->getData(viewer->getDrawScaleFactor())
   );

   // If we are intersecting an object but not grabbing it and the grab
   // button has just been pressed, grab the intersected object.
   if ( mIntersecting && ! mGrabbing &&
        mGrabBtn.test(mWandInterface, gadget::Digital::TOGGLE_ON) )
   {
      mGrabSound.trigger();
      mGrabbing = true;
      mGrabbedObj = mIntersectedObj;

      gmtl::set(mGrabbed_pobj_M_obj, mGrabbedObj->getPos());
   
      std::for_each(mMoveStrategies.begin(), mMoveStrategies.end(),
                    boost::bind(&inf::MoveStrategy::objectGrabbed, _1,
                                viewer, mGrabbedObj, mIntersectPoint,
                                vp_M_wand_xform));

      // Send a select event.
      mEventData->mObjectSelectedSignal(mGrabbedObj);
   }
   // If we are grabbing an object and the grab button has just been pressed
   // again, release the grabbed object.
   else if ( mGrabbing &&
             mGrabBtn.test(mWandInterface, gadget::Digital::TOGGLE_ON) )
   {
      // We have just released the grabbed object, but we are still
      // intersecting it.  Set the bounding box state back to the
      // intersecting state and clear mGrabbedObj.
      if ( mGrabbedObj != NULL && mGrabbedObj->getRoot() != OSG::NullFC )
      {
         gmtl::identity(mGrabbed_pobj_M_obj);

         std::for_each(mMoveStrategies.begin(), mMoveStrategies.end(),
                       boost::bind(&inf::MoveStrategy::objectReleased, _1,
                                   viewer, mGrabbedObj));

         // Send a select event.
         mEventData->mObjectDeselectedSignal(mGrabbedObj);
      }

      mGrabbing = false;
      mGrabbedObj = SceneObjectPtr();
   }
}

void GrabPlugin::run(inf::ViewerPtr viewer)
{
   // Move the grabbed object.
   if ( mGrabbing )
   {
      vprASSERT(NULL != mGrabbedObj.get() &&
                "GrabbedObj can not be NULL if we are grabbing an object.");

      if ( !mMoveStrategies.empty() )
      {
         // Get the wand transformation in virtual world coordinates.
         const gmtl::Matrix44f vp_M_wand_xform(
            mWandInterface->getWandPos()->getData(viewer->getDrawScaleFactor())
         );

         gmtl::Matrix44f new_obj_mat = mGrabbed_pobj_M_obj;

         for (std::vector<MoveStrategyPtr>::iterator itr = mMoveStrategies.begin();
              itr != mMoveStrategies.end(); ++itr)
         {
            // new_obj_mat is gaurenteed to place the object into pobj space
            new_obj_mat = (*itr)->computeMove(viewer, mGrabbedObj, vp_M_wand_xform, new_obj_mat);
         }

         // Send a move event.
         mEventData->mObjectMovedSignal(mGrabbedObj, new_obj_mat);
      }
   }
}

inf::Event::ResultType
GrabPlugin::defaultObjectMovedSlot(SceneObjectPtr obj,
                                   const gmtl::Matrix44f& newObjMat)
{

   OSG::Matrix obj_mat_osg;
   gmtl::set(obj_mat_osg, newObjMat);
   obj->moveTo(obj_mat_osg);
   
   return Event::CONTINUE;
}

bool GrabPlugin::config(jccl::ConfigElementPtr elt)
{
   vprASSERT(elt->getID() == getElementType());

   const unsigned int req_cfg_version(3);

   // Check for correct version of plugin configuration.
   if ( elt->getVersion() < req_cfg_version )
   {
      std::stringstream msg;
      msg << "Configuration of GrabPlugin failed.  Required config "
          << "element version is " << req_cfg_version << ", but element '"
          << elt->getName() << "' is version " << elt->getVersion();
      throw PluginException(msg.str(), IOV_LOCATION);
   }

   const std::string grab_btn_prop("grab_button_nums");
   const std::string strategy_plugin_path_prop("strategy_plugin_path");
   const std::string move_strategy_prop("move_strategy");

   mGrabBtn.configButtons(elt->getProperty<std::string>(grab_btn_prop));

   // Set up two default search paths:
   //    1. Relative path to './plugins/grab'
   //    2. IOV_BASE_DIR/lib/IOV/plugins/grab
   mStrategyPluginPath.push_back("plugins/grab");

   std::string iov_base_dir;
   if ( vpr::System::getenv("IOV_BASE_DIR", iov_base_dir).success() )
   {
      fs::path iov_base_path(iov_base_dir, fs::native);
      fs::path def_strategy_path = iov_base_path / "lib/IOV/plugins/grab";

      if ( fs::exists(def_strategy_path) )
      {
         std::string def_search_path =
            def_strategy_path.native_directory_string();
         std::cout << "Setting default IOV grab strategy plug-in path: " << def_search_path
                   << std::endl;
         mStrategyPluginPath.push_back(def_search_path);
      }
      else
      {
         std::cerr << "Default IOV grab strategy plug-in path does not exist: "
                   << def_strategy_path.native_directory_string()
                   << std::endl;
      }
   }

   const unsigned int num_plugin_paths(elt->getNum(strategy_plugin_path_prop));
   for ( unsigned int i = 0; i < num_plugin_paths; ++i )
   {
      std::string dir = elt->getProperty<std::string>(strategy_plugin_path_prop, i);
      mStrategyPluginPath.push_back(vpr::replaceEnvVars(dir));
   }

   const unsigned int num_move_strategy_names(elt->getNum(move_strategy_prop));
   for ( unsigned int i = 0; i < num_move_strategy_names; ++i)
   {
      std::string move_strategy_name = elt->getProperty<std::string>(move_strategy_prop, i);
      mMoveStrategyNames.push_back(move_strategy_name);
   }

   return true;
}

GrabPlugin::GrabPlugin()
   : mGrabText("Grab/Release Toggle")
   , mIntersecting(false)
   , mGrabbing(false)
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

void GrabPlugin::focusChanged(inf::ViewerPtr viewer)
{
   // If we have focus and our grab/release button is configured, we
   // will update the status panel to include our command.
   if ( isFocused() && mGrabBtn.isConfigured() )
   {
      inf::ScenePtr scene = viewer->getSceneObj();
      StatusPanelPluginDataPtr status_panel_data =
         scene->getSceneData<StatusPanelPluginData>();

      if ( status_panel_data->mStatusPanelPlugin )
      {
         inf::StatusPanel& panel =
            status_panel_data->mStatusPanelPlugin->getPanel();
         if ( ! panel.hasControlText(mGrabBtn.getButtons(), mGrabText) )
         {
            // The button numbers in mGrabBtn are zero-based, but we would like
            // them to be one-based in the status panel display.
            std::vector<int> btns(mGrabBtn.getButtons().size());
            IncValue inc;
            std::transform(mGrabBtn.getButtons().begin(),
                           mGrabBtn.getButtons().end(), btns.begin(), inc);

            panel.addControlText(btns, mGrabText);
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

         // The button numbers in mGrabBtn are zero-based, but we would like
         // them to be one-based in the status panel display.
         std::vector<int> btns(mGrabBtn.getButtons().size());
         IncValue inc;
         std::transform(mGrabBtn.getButtons().begin(),
                        mGrabBtn.getButtons().end(), btns.begin(), inc);

         panel.removeControlText(btns, mGrabText);
      }
   }
}

}
