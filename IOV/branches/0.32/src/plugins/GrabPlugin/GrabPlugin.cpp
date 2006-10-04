// Copyright (C) Infiscape Corporation 2005-2006

#include <string.h>
#include <algorithm>
#include <utility>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
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
#include <IOV/SceneObject.h>
#include <IOV/Status.h>
#include <IOV/Grab/GrabStrategy.h>
#include <IOV/Grab/MoveStrategy.h>
#include <IOV/Util/Exceptions.h>
#include <IOV/Util/OpenSGHelpers.h>

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

GrabPlugin::GrabPlugin()
{
   /* Do nothing. */ ;
}

GrabPlugin::~GrabPlugin()
{
   /* Do nothing. */ ;
}

PluginPtr GrabPlugin::init(ViewerPtr viewer)
{
   mEventData = viewer->getSceneObj()->getSceneData<EventData>();
   mEventData->mObjectsMovedSignal.connect(
      100, boost::bind(&GrabPlugin::defaultObjectsMovedSlot, this, _1)
   );

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

   // Load the grab strategy.
   try
   {
      IOV_STATUS << "   Loading grab strategy plug-in '" << mGrabStrategyName
                 << "' ..." << std::flush;
      inf::PluginCreator<inf::GrabStrategy>* creator =
         mPluginFactory->getPluginCreator<inf::GrabStrategy>(mGrabStrategyName);

      if ( NULL != creator )
      {
         GrabStrategyPtr grab_strategy = creator->createPlugin();
         grab_strategy->init(viewer,
                             boost::bind(&GrabPlugin::objectsGrabbed, this,
                                         viewer, _1, _2),
                             boost::bind(&GrabPlugin::objectsReleased, this,
                                         viewer, _1));
         mGrabStrategy = grab_strategy;
         IOV_STATUS << "[OK]" << std::endl;
      }
      else
      {
         IOV_STATUS << "[ERROR]\nWARNING: No creator for strategy plug-in "
                    << mGrabStrategyName << std::endl;
      }
   }
   catch (std::runtime_error& ex)
   {
      IOV_STATUS << "[ERROR]\nWARNING: Failed to load strategy plug-in "
                 << mGrabStrategyName << ":\n" << ex.what() << std::endl;
   }

   // Build the list of move strategies.
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

void GrabPlugin::update(ViewerPtr viewer)
{
   if ( isFocused() && mGrabStrategy )
   {
      std::vector<SceneObjectPtr> new_grabbed_objs;
      gmtl::Point3f isect_point;
      mGrabStrategy->update(viewer);
   }

   if ( mGrabStrategy )
   {
      // Get the wand transformation in virtual platform coordinates.
      const gmtl::Matrix44f vp_M_wand_xform(
         mWandInterface->getWandPos()->getData(viewer->getDrawScaleFactor())
      );

      const std::vector<SceneObjectPtr> objs =
         mGrabStrategy->getGrabbedObjects();

      // Move the grabbed objects.
      if ( ! objs.empty() && ! mMoveStrategies.empty() )
      {
         std::vector< std::pair<SceneObjectPtr, gmtl::Matrix44f> > move_data;

         std::vector<SceneObjectPtr>::const_iterator o;
         for ( o = objs.begin(); o != objs.end(); ++o )
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
}

inf::Event::ResultType GrabPlugin::
defaultObjectsMovedSlot(const EventData::moved_obj_list_t& objs)
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

   const unsigned int req_cfg_version(5);

   // Check for correct version of plugin configuration.
   if ( elt->getVersion() < req_cfg_version )
   {
      std::stringstream msg;
      msg << "Configuration of GrabPlugin failed.  Required config "
          << "element version is " << req_cfg_version << ", but element '"
          << elt->getName() << "' is version " << elt->getVersion();
      throw PluginException(msg.str(), IOV_LOCATION);
   }

   const std::string strategy_plugin_path_prop("strategy_plugin_path");
   const std::string grab_strategy_prop("grab_strategy");
   const std::string move_strategy_prop("move_strategy");

   // Set up two default search paths:
   //    1. Relative path to './plugins/grab'
   //    2. IOV_BASE_DIR/lib/IOV/plugins/grab
   mStrategyPluginPath.push_back("plugins/grab");

   std::string iov_base_dir;
   vpr::System::getenv("IOV_BASE_DIR", iov_base_dir);
   
   if ( ! iov_base_dir.empty() )
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

   mGrabStrategyName = elt->getProperty<std::string>(grab_strategy_prop);

   const unsigned int num_move_strategy_names(elt->getNum(move_strategy_prop));
   for ( unsigned int i = 0; i < num_move_strategy_names; ++i)
   {
      std::string move_strategy_name = elt->getProperty<std::string>(move_strategy_prop, i);
      mMoveStrategyNames.push_back(move_strategy_name);
   }

   return true;
}

void GrabPlugin::focusChanged(inf::ViewerPtr viewer)
{
   if ( mGrabStrategy )
   {
      mGrabStrategy->setFocus(viewer, isFocused());
   }
}

void GrabPlugin::objectsGrabbed(inf::ViewerPtr viewer,
                                const std::vector<SceneObjectPtr>& objs,
                                const gmtl::Point3f& isectPoint)
{
   std::vector<SceneObjectPtr>::const_iterator o;
   for ( o = objs.begin(); o != objs.end(); ++o )
   {
      gmtl::set(mGrabbed_pobj_M_obj_map[*o], (*o)->getPos());
   }

   // Get the wand transformation in virtual platform coordinates.
   const gmtl::Matrix44f vp_M_wand_xform(
      mWandInterface->getWandPos()->getData(viewer->getDrawScaleFactor())
   );

   std::for_each(mMoveStrategies.begin(), mMoveStrategies.end(),
                 boost::bind(&MoveStrategy::objectsGrabbed, _1, viewer, objs,
                             isectPoint, vp_M_wand_xform));

   // Emit the object selection signal.
   mEventData->mObjectsSelectedSignal(objs);
}

void GrabPlugin::objectsReleased(inf::ViewerPtr viewer,
                                 const std::vector<SceneObjectPtr>& objs)
{
   std::vector<SceneObjectPtr>::const_iterator o;
   for ( o = objs.begin(); o != objs.end(); ++o )
   {
      mGrabbed_pobj_M_obj_map.erase(*o);
   }

   std::for_each(mMoveStrategies.begin(), mMoveStrategies.end(),
                 boost::bind(&MoveStrategy::objectsReleased, _1, viewer,
                             objs));

   // Emit the de-select event for all the objects that were released.
   mEventData->mObjectsDeselectedSignal(objs);
}

}
