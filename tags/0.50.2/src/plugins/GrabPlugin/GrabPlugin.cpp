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

#include <cstring>
#include <algorithm>
#include <utility>
#include <boost/bind.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/assign/list_of.hpp>

#include <gmtl/Matrix.h>
#include <gmtl/MatrixOps.h>
#include <gmtl/External/OpenSGConvert.h>

#include <vpr/vpr.h>
#include <vpr/Util/FileUtils.h>

#include <vrkit/Viewer.h>
#include <vrkit/User.h>
#include <vrkit/InterfaceTrader.h>
#include <vrkit/WandInterface.h>
#include <vrkit/SceneObject.h>
#include <vrkit/Status.h>
#include <vrkit/Exception.h>
#include <vrkit/Version.h>
#include <vrkit/exceptions/PluginException.h>
#include <vrkit/scenedata/EventData.h>
#include <vrkit/plugin/Creator.h>
#include <vrkit/plugin/Registry.h>
#include <vrkit/plugin/TypedRegistryEntry.h>
#include <vrkit/plugin/Info.h>
#include <vrkit/plugin/Helpers.h>
#include <vrkit/grab/Strategy.h>
#include <vrkit/move/Strategy.h>
#include <vrkit/util/OpenSGHelpers.h>

#include "GrabPlugin.h"


using namespace boost::assign;
namespace fs = boost::filesystem;

static const vrkit::plugin::Info sInfo(
   "com.infiscape", "GrabPlugin",
   list_of(VRKIT_VERSION_MAJOR)(VRKIT_VERSION_MINOR)(VRKIT_VERSION_PATCH)
);
static vrkit::plugin::Creator<vrkit::viewer::Plugin> sPluginCreator(
   boost::bind(&vrkit::GrabPlugin::create, sInfo)
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

template<typename T>
void registerModule(vpr::LibraryPtr module, ViewerPtr viewer)
{
   viewer->getPluginRegistry()->addEntry(
      plugin::TypedRegistryEntry<T>::create(module, &T::validatePluginLib)
   );
}

GrabPlugin::GrabPlugin(const plugin::Info& info)
   : viewer::Plugin(info)
{
   /* Do nothing. */ ;
}

GrabPlugin::~GrabPlugin()
{
   /* Do nothing. */ ;
}

viewer::PluginPtr GrabPlugin::init(ViewerPtr viewer)
{
   mEventData = viewer->getSceneObj()->getSceneData<EventData>();
   mEventData->objectsMoved.connect(
      100, boost::bind(&GrabPlugin::defaultObjectsMovedSlot, this, _1)
   );

   InterfaceTrader& if_trader = viewer->getUser()->getInterfaceTrader();
   mWandInterface = if_trader.getWandInterface();

   // Get the plug-in registry for our strategy plug-in types.
   plugin::RegistryPtr plugin_registry = viewer->getPluginRegistry();
   plugin_registry->pluginInstantiated().connect(
      boost::bind(&GrabPlugin::pluginInstantiated, this, _1, viewer)
   );

   // Configure
   std::string elt_type_name = getElementType();
   jccl::ConfigElementPtr cfg_elt =
      viewer->getConfiguration().getConfigElement(elt_type_name);

   if ( cfg_elt )
   {
      // Configure it
      config(cfg_elt, viewer);
   }

   // Load the grab strategy.
   try
   {
      VRKIT_STATUS << "   Loading grab strategy plug-in '"
                   << mGrabStrategyName << "' ..." << std::flush;
      std::vector<AbstractPluginPtr> deps;
      plugin_registry->makeInstance(mGrabStrategyName, deps);
      VRKIT_STATUS << "[OK]" << std::endl;
   }
   catch (std::runtime_error& ex)
   {
      VRKIT_STATUS << "[ERROR]\nWARNING: Failed to load strategy plug-in "
                   << mGrabStrategyName << ":\n" << ex.what() << std::endl;
   }

   // Build the list of move strategies.
   typedef std::vector<std::string>::iterator iter_type;
   for ( iter_type itr = mMoveStrategyNames.begin();
         itr != mMoveStrategyNames.end();
         ++itr )
   {
      try
      {
         VRKIT_STATUS << "   Loading move strategy plug-in '" << *itr
                      << "' ..." << std::flush;
         std::vector<AbstractPluginPtr> deps;
         plugin_registry->makeInstance(*itr, deps);
         VRKIT_STATUS << "[OK]" << std::endl;
      }
      catch (std::runtime_error& ex)
      {
         VRKIT_STATUS << "[ERROR]\nWARNING: Failed to load strategy plug-in "
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

            typedef std::vector<move::StrategyPtr>::iterator iter_type;
            for ( iter_type s = mMoveStrategies.begin();
                  s != mMoveStrategies.end();
                  ++s )
            {
               // new_obj_mat is guaranteed to place the object into pobj
               // space.
               new_obj_mat = (*s)->computeMove(viewer, *o, vp_M_wand_xform,
                                               new_obj_mat);
            }

            move_data.push_back(std::make_pair(*o, new_obj_mat));
         }

         // Send a move event.
         mEventData->objectsMoved(move_data);
      }
   }
}

void GrabPlugin::focusChanged(ViewerPtr viewer)
{
   if ( mGrabStrategy )
   {
      mGrabStrategy->setFocus(viewer, isFocused());
   }
}

event::ResultType
GrabPlugin::defaultObjectsMovedSlot(const EventData::moved_obj_list_t& objs)
{
   EventData::moved_obj_list_t::const_iterator o;
   for ( o = objs.begin(); o != objs.end(); ++o )
   {
      OSG::Matrix obj_mat_osg;
      gmtl::set(obj_mat_osg, (*o).second);
      (*o).first->moveTo(obj_mat_osg);
   }

   return event::CONTINUE;
}

bool GrabPlugin::config(jccl::ConfigElementPtr elt, ViewerPtr viewer)
{
   vprASSERT(elt->getID() == getElementType());

   const unsigned int req_cfg_version(6);

   // Check for correct version of plugin configuration.
   if ( elt->getVersion() < req_cfg_version )
   {
      std::stringstream msg;
      msg << "Configuration of GrabPlugin failed.  Required config "
          << "element version is " << req_cfg_version << ", but element '"
          << elt->getName() << "' is version " << elt->getVersion();
      throw PluginException(msg.str(), VRKIT_LOCATION);
   }

   const std::string grab_strategy_path_prop("grab_strategy_plugin_path");
   const std::string move_strategy_path_prop("move_strategy_plugin_path");
   const std::string grab_strategy_prop("grab_strategy");
   const std::string move_strategy_prop("move_strategy");

   // Set up two default search paths for Grab Strategy plug-ins:
   //    1. Relative path to './plugins/grab'
   //    2. VRKIT_BASE_DIR/lib/vrkit/plugins/grab
   //
   // In all of the above cases, the 'debug' subdirectory is searched first if
   // this is a debug build (i.e., when VRKIT_DEBUG is defined and _DEBUG is
   // not).
   const std::vector<std::string> grab_search_path =
      makeSearchPath(elt, grab_strategy_path_prop, "grab");

   // Set up two default search paths for Move Strategy plug-ins:
   //    1. Relative path to './plugins/move'
   //    2. VRKIT_BASE_DIR/lib/vrkit/plugins/move
   //
   // In all of the above cases, the 'debug' subdirectory is searched first if
   // this is a debug build (i.e., when VRKIT_DEBUG is defined and _DEBUG is
   // not).
   const std::vector<std::string> move_search_path =
      makeSearchPath(elt, move_strategy_path_prop, "move");

   mGrabStrategyName = elt->getProperty<std::string>(grab_strategy_prop);

   const unsigned int num_move_strategy_names(elt->getNum(move_strategy_prop));
   for ( unsigned int i = 0; i < num_move_strategy_names; ++i)
   {
      mMoveStrategyNames.push_back(
         elt->getProperty<std::string>(move_strategy_prop, i)
      );
   }

   std::vector<vpr::LibraryPtr> modules =
      plugin::findModules(grab_search_path);
   std::for_each(modules.begin(), modules.end(),
                 boost::bind(&registerModule<grab::Strategy>, _1, viewer));

   modules = plugin::findModules(move_search_path);
   std::for_each(modules.begin(), modules.end(),
                 boost::bind(&registerModule<move::Strategy>, _1, viewer));

   return true;
}

std::vector<std::string>
GrabPlugin::makeSearchPath(jccl::ConfigElementPtr elt,
                           const std::string& prop, const std::string& subdir)
{
   std::vector<std::string> search_path =
      plugin::getDefaultSearchPath(subdir);

   const unsigned int num_plugin_paths(elt->getNum(prop));

   if ( num_plugin_paths > 0 )
   {
      search_path.reserve(search_path.size() + num_plugin_paths);
   }

   for ( unsigned int i = 0; i < num_plugin_paths; ++i )
   {
      std::string dir = elt->getProperty<std::string>(prop, i);
      search_path.push_back(vpr::replaceEnvVars(dir));
   }

   return search_path;
}

void GrabPlugin::pluginInstantiated(AbstractPluginPtr plugin,
                                    ViewerPtr viewer)
{
   if ( ! mGrabStrategy )
   {
      grab::StrategyPtr g =
         boost::dynamic_pointer_cast<grab::Strategy>(plugin);

      if ( g )
      {
         mGrabStrategy = g;
         mGrabStrategy->init(viewer,
                             boost::bind(&GrabPlugin::objectsGrabbed, this,
                                         viewer, _1, _2),
                             boost::bind(&GrabPlugin::objectsReleased, this,
                                         viewer, _1));
         return;
      }
   }

   move::StrategyPtr m = boost::dynamic_pointer_cast<move::Strategy>(plugin);

   if ( m )
   {
      m->init(viewer);
      mMoveStrategies.push_back(m);
   }
}

void GrabPlugin::objectsGrabbed(ViewerPtr viewer,
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
                 boost::bind(&move::Strategy::objectsGrabbed, _1, viewer,
                             objs, isectPoint, vp_M_wand_xform));

   // Emit the object selection signal.
   mEventData->objectsSelected(objs);
}

void GrabPlugin::objectsReleased(ViewerPtr viewer,
                                 const std::vector<SceneObjectPtr>& objs)
{
   std::vector<SceneObjectPtr>::const_iterator o;
   for ( o = objs.begin(); o != objs.end(); ++o )
   {
      mGrabbed_pobj_M_obj_map.erase(*o);
   }

   std::for_each(mMoveStrategies.begin(), mMoveStrategies.end(),
                 boost::bind(&move::Strategy::objectsReleased, _1, viewer,
                             objs));

   // Emit the de-select event for all the objects that were released.
   mEventData->objectsDeselected(objs);
}

}
