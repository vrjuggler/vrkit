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

#include <IOV/Viewer.h>
#include <IOV/PluginCreator.h>
#include <IOV/User.h>
#include <IOV/InterfaceTrader.h>
#include <IOV/WandInterface.h>
#include <IOV/ViewPlatform.h>
#include <IOV/Util/Exceptions.h>
#include <IOV/Status.h>
#include <IOV/Version.h>
#include <IOV/Plugin/Info.h>

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

static const inf::plugin::Info sInfo(
   "com.infiscape", "ViewpointsPlugin",
   list_of(IOV_VERSION_MAJOR)(IOV_VERSION_MINOR)(IOV_VERSION_PATCH)
);
static inf::PluginCreator<inf::Plugin> sViewpointsPluginCreator(
   boost::bind(&inf::ViewpointsPlugin::create, sInfo)
);

extern "C"
{

/** @name Plug-in Entry points */
//@{
IOV_PLUGIN_API(const inf::plugin::Info*) getPluginInfo()
{
   return &sInfo;
}

IOV_PLUGIN_API(void) getPluginInterfaceVersion(vpr::Uint32& majorVer,
                                               vpr::Uint32& minorVer)
{
   majorVer = INF_PLUGIN_API_MAJOR;
   minorVer = INF_PLUGIN_API_MINOR;
}

IOV_PLUGIN_API(inf::PluginCreatorBase*) getCreator()
{
   return &sViewpointsPluginCreator;
}
//@}

}

namespace inf
{

PluginPtr ViewpointsPlugin::create(const inf::plugin::Info& info)
{
   return PluginPtr(new ViewpointsPlugin(info));
}

PluginPtr ViewpointsPlugin::init(inf::ViewerPtr viewer)
{
   const unsigned int req_cfg_version(2);

   // Get the wand interface
   InterfaceTrader& if_trader = viewer->getUser()->getInterfaceTrader();
   mWandInterface = if_trader.getWandInterface();

   jccl::ConfigElementPtr elt = viewer->getConfiguration().getConfigElement(vp_plugin_elt_tkn);

   if(!elt)
   {
      std::stringstream ex_msg;
      ex_msg << "Viewpoint plugin could not find its configuration.  "
             << "Looking for type: " << vp_plugin_elt_tkn;
      throw PluginException(ex_msg.str(), IOV_LOCATION);
   }

   // -- Read configuration -- //
   vprASSERT(elt->getID() == vp_plugin_elt_tkn);

   // Check for correct version of plugin configuration
   if(elt->getVersion() < req_cfg_version)
   {
      std::stringstream msg;
      msg << "ModeSwitchPlugin: Configuration failed. Required cfg version: " << req_cfg_version
          << " found:" << elt->getVersion();
      throw PluginException(msg.str(), IOV_LOCATION);
   }

   // Get the control button(s) to use.
   mControlCmd.configure(
      elt->getProperty<std::string>(vp_control_command_exp_tkn),
      mWandInterface
   );

   float to_meters_scalar = elt->getProperty<float>(vp_units_to_meters_tkn);

   // Read in all the viewpoints
   unsigned num_vps = elt->getNum(vp_viewpoints_tkn);
   for(unsigned i=0;i<num_vps;i++)
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
      vp_coord.pos().set(xt,yt,zt);
      vp_coord.rot().set(gmtl::Math::deg2Rad(xr),
                         gmtl::Math::deg2Rad(yr),
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
void ViewpointsPlugin::update(inf::ViewerPtr viewer)
{
   if ( isFocused() )
   {
      // If we have viewpoints and the button has been pressed
      if ( ! mViewpoints.empty() && mControlCmd() )
      {
         vprASSERT(mNextViewpoint < mViewpoints.size());
         Viewpoint vp = mViewpoints[mNextViewpoint];

         IOV_STATUS << boost::format("Selecting new viewpoint: [%s] %s") % mNextViewpoint % vp.mName
                    << std::endl;

         gmtl::Coord3fXYZ new_coord = gmtl::make<gmtl::Coord3fXYZ>(vp.mXform);
         IOV_STATUS << "   New pos: " << new_coord << std::endl;

         inf::ViewPlatform& viewplatform = viewer->getUser()->getViewPlatform();
         viewplatform.setCurPos(vp.mXform);

         mNextViewpoint += 1;
         if ( mNextViewpoint >= mViewpoints.size() )
         {
            mNextViewpoint = 0;
         }
      }
   }
}

} // namespace inf
