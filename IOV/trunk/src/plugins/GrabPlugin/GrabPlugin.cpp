// Copyright (C) Infiscape Corporation 2005-2006

#include <string.h>
#include <algorithm>
#include <utility>
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
#include <IOV/Grab/MoveStrategy.h>
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

GrabPlugin::~GrabPlugin()
{
   mIsectConnection.disconnect();
   mDeIsectConnection.disconnect();

   std::for_each(
      mGrabbedObjConnections.begin(), mGrabbedObjConnections.end(),
      boost::bind(&boost::signals::connection::disconnect,
                  boost::bind(&grab_conn_map_t::value_type::second, _1))
   );
}

PluginPtr GrabPlugin::init(ViewerPtr viewer)
{
   mEventData = viewer->getSceneObj()->getSceneData<EventData>();
   mEventData->mObjectsMovedSignal.connect(
      100, boost::bind(&GrabPlugin::defaultObjectsMovedSlot, this, _1)
   );

   // Connect the intersection signal to our slot.
   mIsectConnection =
      mEventData->mObjectIntersectedSignal.connect(
         0, boost::bind(&GrabPlugin::objectIntersected, this, _1, _2, _3,
                        viewer)
      );

   // Connect the de-intersection signal to our slot.
   mDeIsectConnection = mEventData->mObjectDeintersectedSignal.connect(0, boost::bind(&GrabPlugin::objectDeintersected, this, _1));

   InterfaceTrader& if_trader = viewer->getUser()->getInterfaceTrader();
   mWandInterface = if_trader.getWandInterface();

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

struct IncValue
{
   int operator()(int v)
   {
      return v + 1;
   }
};

std::vector<int> GrabPlugin::transformButtonVec(const std::vector<int>& btns)
{
   std::vector<int> result(btns.size());
   IncValue inc;
   std::transform(btns.begin(), btns.end(), result.begin(), inc);
   return result;
}

inf::Event::ResultType
GrabPlugin::objectIntersected(inf::SceneObjectPtr obj,
                              inf::SceneObjectPtr parentObj,
                              gmtl::Point3f pnt, inf::ViewerPtr viewer)
{
   if (!mGrabbing)
   {
      // If we intersected a grabbable object.
      if ( obj->isGrabbable() )
      {
         mIntersectedObj = obj;
         mIntersectPoint = pnt;
         mIntersecting = true;

         // Connect the grabbable object state change signal to our slot.
         mGrabbedObjConnections[mIntersectedObj] =
            mIntersectedObj->grabbableStateChanged().connect(
               boost::bind(&GrabPlugin::grabbableObjStateChanged, this, _1,
                           viewer)
            );
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

void GrabPlugin::update(ViewerPtr viewer)
{
   // Get the wand transformation in virtual platform coordinates.
   const gmtl::Matrix44f vp_M_wand_xform(
      mWandInterface->getWandPos()->getData(viewer->getDrawScaleFactor())
   );

   if ( isFocused() )
   {
      // If we are intersecting an object but not grabbing it, the grab
      // button has just been pressed, and the intersected object is grabbable,
      // then we can grab the intersected object.
      if ( mIntersecting && ! mGrabbing && mIntersectedObj->isGrabbable() &&
           mGrabBtn.test(mWandInterface, gadget::Digital::TOGGLE_ON) )
      {
         mGrabbing = true;
         vprASSERT(mGrabbedObjs.empty() &&
                   "mGrabbedObjs should be empty before grabbing");
         mGrabbedObjs.push_back(mIntersectedObj);

         gmtl::set(mGrabbed_pobj_M_obj_map[mIntersectedObj],
                   mIntersectedObj->getPos());
      
         std::for_each(mMoveStrategies.begin(), mMoveStrategies.end(),
                       boost::bind(&inf::MoveStrategy::objectsGrabbed, _1,
                                   viewer, mGrabbedObjs, mIntersectPoint,
                                   vp_M_wand_xform));

         // Send a select event.
         mEventData->mObjectsSelectedSignal(mGrabbedObjs);
      }
      // If we are grabbing an object and the release button has just been
      // pressed, then release the grabbed object.
      else if ( mGrabbing &&
                mReleaseBtn.test(mWandInterface, gadget::Digital::TOGGLE_ON) )
      {
         // Update our state to reflect that we are no longer grabbing an
         // object.
         mGrabbing = false;
         releaseGrabbedObjects(viewer, mGrabbedObjs);
         vprASSERT(mGrabbedObjs.empty());
      }
   }

   // Move the grabbed objects.
   if ( ! mGrabbedObjs.empty() && ! mMoveStrategies.empty() )
   {
      std::vector< std::pair<SceneObjectPtr, gmtl::Matrix44f> > move_data;

      std::vector<SceneObjectPtr>::iterator o;
      for ( o = mGrabbedObjs.begin(); o != mGrabbedObjs.end(); ++o )
      {
         gmtl::Matrix44f new_obj_mat = mGrabbed_pobj_M_obj_map[*o];

         std::vector<MoveStrategyPtr>::iterator s;
         for ( s = mMoveStrategies.begin(); s != mMoveStrategies.end(); ++s )
         {
            // new_obj_mat is guaranteed to place the object into pobj
            // space.
            new_obj_mat = (*s)->computeMove(viewer, *o, vp_M_wand_xform,
                                            new_obj_mat);
         }

         move_data.push_back(std::make_pair(*o, new_obj_mat));
      }

      // Send a move event.
      mEventData->mObjectsMovedSignal(move_data);
   }
}

inf::Event::ResultType
GrabPlugin::defaultObjectsMovedSlot(const EventData::moved_obj_list_t& objs)
{
   EventData::moved_obj_list_t::const_iterator o;
   for ( o = objs.begin(); o != objs.end(); ++o )
   {
      OSG::Matrix obj_mat_osg;
      gmtl::set(obj_mat_osg, (*o).second);
      (*o).first->moveTo(obj_mat_osg);
   }

   return Event::CONTINUE;
}

bool GrabPlugin::config(jccl::ConfigElementPtr elt)
{
   vprASSERT(elt->getID() == getElementType());

   const unsigned int req_cfg_version(4);

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
   const std::string release_btn_prop("release_button_nums");
   const std::string strategy_plugin_path_prop("strategy_plugin_path");
   const std::string move_strategy_prop("move_strategy");

   mGrabBtn.configButtons(elt->getProperty<std::string>(grab_btn_prop));
   mReleaseBtn.configButtons(elt->getProperty<std::string>(release_btn_prop));

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
   : mGrabText("Grab")
   , mReleaseText("Release")
   , mIntersecting(false)
   , mGrabbing(false)
{
   ;
}

void GrabPlugin::focusChanged(inf::ViewerPtr viewer)
{
   // If we have focus, we will try to update the staus panel to include our
   // commands.
   if ( isFocused() )
   {
      inf::ScenePtr scene = viewer->getSceneObj();
      StatusPanelPluginDataPtr status_panel_data =
         scene->getSceneData<StatusPanelPluginData>();

      if ( status_panel_data->mStatusPanelPlugin )
      {
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
         panel.removeControlText(transformButtonVec(mGrabBtn.getButtons()),
                                 mGrabText);

         // The button numbers in mReleaseBtn are zero-based, but we would like
         // them to be one-based in the status panel display.
         panel.removeControlText(transformButtonVec(mReleaseBtn.getButtons()),
                                 mReleaseText);
      }
   }
}

void GrabPlugin::releaseGrabbedObjects(inf::ViewerPtr viewer,
                                       const std::vector<SceneObjectPtr>& objs)
{
   std::vector<SceneObjectPtr>::const_iterator o;
   for ( o = objs.begin(); o != objs.end(); ++o )
   {
      // Remove the released object from mGrabbedObjs.
      std::vector<SceneObjectPtr>::iterator i =
         std::find(mGrabbedObjs.begin(), mGrabbedObjs.end(), *o);
      vprASSERT(i != mGrabbedObjs.end());
      mGrabbedObjs.erase(i);

      // Disconnect our connection to the current object's grabbable state
      // change signal so that any state changes to it do not propagate back
      // to us. We are no longer interested in changes since we have released
      // the object.
      vprASSERT(mGrabbedObjConnections.count(*o) != 0);
      mGrabbedObjConnections[*o].disconnect();
      mGrabbedObjConnections.erase(*o);

      mGrabbed_pobj_M_obj_map.erase(*o);
   }

   std::for_each(mMoveStrategies.begin(), mMoveStrategies.end(),
                 boost::bind(&MoveStrategy::objectsReleased, _1, viewer,
                             objs));

   // Emit the de-select event for all the objects that were released.
   mEventData->mObjectsDeselectedSignal(objs);
}

void GrabPlugin::grabbableObjStateChanged(inf::SceneObjectPtr obj,
                                          inf::ViewerPtr viewer)
{
   std::vector<SceneObjectPtr>::iterator o =
      std::find(mGrabbedObjs.begin(), mGrabbedObjs.end(), obj);

   // If we are currently grabbing obj and its grabbable state has changed so
   // that it is no longer grabbable, we must release it.
   if ( o != mGrabbedObjs.end() && ! obj->isGrabbable() )
   {
      std::vector<SceneObjectPtr> objs(1, obj);
      releaseGrabbedObjects(viewer, objs);
   }
}

}
