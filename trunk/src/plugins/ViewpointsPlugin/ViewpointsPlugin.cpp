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

#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#endif

#include <sstream>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/assign/list_of.hpp>

#include <gmtl/Matrix.h>
#include <gmtl/Vec.h>
#include <gmtl/MatrixOps.h>
#include <gmtl/VecOps.h>
#include <gmtl/Output.h>
#include <gmtl/Generate.h>

#include <vrkit/Viewer.h>
#include <vrkit/User.h>
#include <vrkit/InterfaceTrader.h>
#include <vrkit/WandInterface.h>
#include <vrkit/ViewPlatform.h>
#include <vrkit/Status.h>
#include <vrkit/Version.h>
#include <vrkit/plugin/Info.h>
#include <vrkit/plugin/Creator.h>
#include <vrkit/exceptions/PluginException.h>

#include "ViewpointsPlugin.h"


namespace
{
   const std::string vp_plugin_elt_tkn("viewpoints_plugin");
   const std::string vp_control_command_exp_tkn("control_command_exp");
   const std::string vp_viewpoints_tkn("viewpoints");
   const std::string vp_units_to_meters_tkn("units_to_meters");

   const std::string vp_vp_elt_tkn("viewpoints_vp");
   const std::string vp_pos_elt_tkn("position");
   const std::string vp_rot_elt_tkn("rotation");
}

using namespace boost::assign;

static const vrkit::plugin::Info sInfo(
   "com.infiscape", "ViewpointsPlugin",
   list_of(VRKIT_VERSION_MAJOR)(VRKIT_VERSION_MINOR)(VRKIT_VERSION_PATCH)
);
static vrkit::plugin::Creator<vrkit::viewer::Plugin> sViewpointsPluginCreator(
   boost::bind(&vrkit::ViewpointsPlugin::create, sInfo)
);

extern "C"
{

/** @name Plug-in Entry points */
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
   return &sViewpointsPluginCreator;
}
//@}

}

namespace vrkit
{

viewer::PluginPtr ViewpointsPlugin::create(const plugin::Info& info)
{
   return viewer::PluginPtr(new ViewpointsPlugin(info));
}

viewer::PluginPtr ViewpointsPlugin::init(ViewerPtr viewer)
{
   const unsigned int req_cfg_version(2);

   // Get the wand interface
   InterfaceTrader& if_trader = viewer->getUser()->getInterfaceTrader();
   mWandInterface = if_trader.getWandInterface();

   jccl::ConfigElementPtr elt =
      viewer->getConfiguration().getConfigElement(vp_plugin_elt_tkn);

   if ( ! elt )
   {
      std::stringstream ex_msg;
      ex_msg << "Viewpoint plugin could not find its configuration.  "
             << "Looking for type: " << vp_plugin_elt_tkn;
      throw PluginException(ex_msg.str(), VRKIT_LOCATION);
   }

   // -- Read configuration -- //
   vprASSERT(elt->getID() == vp_plugin_elt_tkn);

   // Check for correct version of plugin configuration
   if ( elt->getVersion() < req_cfg_version )
   {
      std::stringstream msg;
      msg << "ViewpointPlugin: Configuration failed. Required cfg version: "
          << req_cfg_version << " found:" << elt->getVersion();
      throw PluginException(msg.str(), VRKIT_LOCATION);
   }

   // Get the control button(s) to use.
   mControlCmd.configure(
      elt->getProperty<std::string>(vp_control_command_exp_tkn),
      mWandInterface
   );

   float to_meters_scalar = elt->getProperty<float>(vp_units_to_meters_tkn);

   // Read in all the viewpoints
   unsigned int num_vps = elt->getNum(vp_viewpoints_tkn);
   for ( unsigned int i = 0; i < num_vps; ++i )
   {
      jccl::ConfigElementPtr vp_elt =
         elt->getProperty<jccl::ConfigElementPtr>(vp_viewpoints_tkn, i);
      vprASSERT(vp_elt.get() != NULL);
      float xt = vp_elt->getProperty<float>(vp_pos_elt_tkn, 0);
      float yt = vp_elt->getProperty<float>(vp_pos_elt_tkn, 1);
      float zt = vp_elt->getProperty<float>(vp_pos_elt_tkn, 2);
      xt *= to_meters_scalar;
      yt *= to_meters_scalar;
      zt *= to_meters_scalar;

      float xr = vp_elt->getProperty<float>(vp_rot_elt_tkn, 0);
      float yr = vp_elt->getProperty<float>(vp_rot_elt_tkn, 1);
      float zr = vp_elt->getProperty<float>(vp_rot_elt_tkn, 2);

      gmtl::Coord3fXYZ vp_coord;
      vp_coord.pos().set(xt, yt, zt);
      vp_coord.rot().set(gmtl::Math::deg2Rad(xr), gmtl::Math::deg2Rad(yr),
                         gmtl::Math::deg2Rad(zr));

      Viewpoint vp;
      vp.mXform = gmtl::make<gmtl::Matrix44f>(vp_coord); // Set at T*R
      vp.mName = vp_elt->getName();
      mViewpoints.push_back(vp);
   }
   vprASSERT(mViewpoints.size() == num_vps);

   // Setup the internals
   // - Setup the next viewpoint to use
   /*
   if(num_vps > 0)
   {
      mNextViewpoint = 1;
   }
   */

   return shared_from_this();
}

//
// Update the state of the plugin
// - If there are viewpoints and the button is pressed
//    - Get the new transform
//    - Set it on the viewplatform
//    - Get ready for the next location
//
void ViewpointsPlugin::update(ViewerPtr viewer)
{
   if ( isFocused() )
   {
      // If we have viewpoints and the button has been pressed
      if ( ! mViewpoints.empty() && mControlCmd() )
      {
         vprASSERT(mNextViewpoint < mViewpoints.size());
         Viewpoint vp = mViewpoints[mNextViewpoint];

         VRKIT_STATUS
            << boost::format("Selecting new viewpoint: [%s] %s") % mNextViewpoint % vp.mName
            << std::endl;

         gmtl::Coord3fXYZ new_coord = gmtl::make<gmtl::Coord3fXYZ>(vp.mXform);
         VRKIT_STATUS << "   New pos: " << new_coord << std::endl;

         ViewPlatform& viewplatform = viewer->getUser()->getViewPlatform();
         viewplatform.setCurPos(vp.mXform);

         mNextViewpoint += 1;
         if ( mNextViewpoint >= mViewpoints.size() )
         {
            mNextViewpoint = 0;
         }
      }
   }
}

} // namespace vrkit
