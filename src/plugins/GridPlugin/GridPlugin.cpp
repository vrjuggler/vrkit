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

#include <vrkit/Scene.h>
#include <vrkit/Status.h>
#include <vrkit/User.h>
#include <vrkit/Viewer.h>
#include <vrkit/WandInterface.h>
#include <vrkit/Version.h>
#include <vrkit/scenedata/StatusPanelData.h>
#include <vrkit/plugin/Creator.h>
#include <vrkit/plugin/Info.h>
#include <vrkit/exceptions/PluginException.h>

#include "Grid.h"
#include "GridPlugin.h"


using namespace boost::assign;

static const vrkit::plugin::Info sInfo(
   "com.infiscape", "GridPlugin",
   list_of(VRKIT_VERSION_MAJOR)(VRKIT_VERSION_MINOR)(VRKIT_VERSION_PATCH)
);
static vrkit::plugin::Creator<vrkit::viewer::Plugin> sPluginCreator(
   boost::bind(&vrkit::GridPlugin::create, sInfo)
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

GridPlugin::GridPlugin(const plugin::Info& info)
   : viewer::Plugin(info)
   , mSelectedGridIndex(-1)
   , mGridsVisible(false)
   , mAnalogNum(-1)
   , mForwardVal(1.0f)
   , mActivateText("Activate/Deactivate Grids")
   , mCycleText("Cycle Grid Selection")
   , mHideText("Show/Hide Selected Grid")
   , mResetText("Reset Selected Grid Position")
{
   /* Do nothing. */ ;
}

viewer::PluginPtr GridPlugin::init(ViewerPtr viewer)
{
   mWandInterface = viewer->getUser()->getInterfaceTrader().getWandInterface();

   jccl::ConfigElementPtr cfg_elt =
      viewer->getConfiguration().getConfigElement(getElementType());

   if ( cfg_elt )
   {
      configure(cfg_elt);
   }

   typedef std::vector<GridPtr>::iterator iter_type;
   for ( iter_type g = mGrids.begin(); g != mGrids.end(); ++g )
   {
      OSG::GroupNodePtr decorator_root =
         viewer->getSceneObj()->getDecoratorRoot();
#if OSG_MAJOR_VERSION < 2
      OSG::CPEditor dre(decorator_root.node(), OSG::Node::ChildrenFieldMask);
#endif
      decorator_root.node()->addChild((*g)->getRoot());
      (*g)->setVisible(mGridsVisible);
   }

   return shared_from_this();
}

void GridPlugin::update(ViewerPtr viewer)
{
   if ( isFocused() )
   {
      // Change the visiblity of all the grids of the activate/deactivate
      // button is toggled on.
      if ( mActivateBtn() )
      {
         mGridsVisible = ! mGridsVisible;
         typedef std::vector<GridPtr>::iterator iter_type;
         for ( iter_type g = mGrids.begin(); g != mGrids.end(); ++g )
         {
            (*g)->setVisible(mGridsVisible);
         }
      }

      // Change the grid selection state if the cycle button has toggled on.
      if ( mCycleBtn() )
      {
         if ( mSelectedGridIndex != -1 )
         {
            mGrids[mSelectedGridIndex]->setSelected(false);
         }

         // If there are grids defined, move mSelectedGridIndex to the next one
         // (using a circular increment) and tell that grid that it is now
         // selected.
         if ( ! mGrids.empty() )
         {
            mSelectedGridIndex = (mSelectedGridIndex + 1) % mGrids.size();
            mGrids[mSelectedGridIndex]->setSelected(true);

            VRKIT_STATUS << "Grid #" << mSelectedGridIndex
                         << " (" << mGrids[mSelectedGridIndex]->getName()
                         << ") selected" << std::endl;

            // XXX: Should the visible state change when the grid is selected?
            // If it doesn't, then there is no visual cue to the user that the
            // grid has been selected. On the other hand, if the user is just
            // trying to cycle to some other grid (visible or not), it may not
            // be desirable to make invisible grids along the way visible.
//            mGrids[mSelectedGridIndex]->setVisible(true);
         }
      }

      // If the user has selected a grid, check to see if any manipulations are
      // being performed on it.
      if ( mSelectedGridIndex != -1 )
      {
         float in_out_val(0.0f);

         // If an analog device index is configured, check to see if analog
         // data is available that would result in the selected grid being
         // slid along its Z-axis.
         if ( -1 != mAnalogNum )
         {
            gadget::AnalogInterface& analog_dev =
               mWandInterface->getAnalog(mAnalogNum);

            // Only check for analog data if the indicated analog device is not
            // stupefied.
            if ( ! analog_dev->isStupefied() )
            {
               const float to_meters(viewer->getDrawScaleFactor());
               const float analog_value(analog_dev->getData());

               // Rescale [0,1] to [-1,1].
               in_out_val = analog_value * 2.0 - 1.0f;

               const float eps_limit(0.1f * to_meters);

               if ( gmtl::Math::abs(in_out_val) < eps_limit )
               {
                  in_out_val = 0.0f;
               }

               // The above code treats the forward value as 1.0. If the
               // forward value is 1.0, then we need to invert the sliding
               // direction.
               if ( mForwardVal == 0.0f )
               {
                  in_out_val = -in_out_val;
               }
            }
         }

         // If in_out_val is not zero, then slide the selected grid along its
         // Z-axis based on in_out_val.
         if ( in_out_val != 0.0f )
         {
            const float in_out_scale(0.20f);
            const float trans_val(-in_out_val * in_out_scale);

            const gmtl::Matrix44f delta_trans_mat =
               gmtl::makeTrans<gmtl::Matrix44f>(gmtl::Vec3f(0.0f, 0.0f,
                                                            trans_val));

            GridPtr grid = mGrids[mSelectedGridIndex];
            gmtl::Matrix44f grid_xform;
            gmtl::set(grid_xform, grid->getCurrentXform());
            grid->move(grid_xform * delta_trans_mat);
         }
         // If no sliding is being performed and the reset button is on, then
         // reset the position of the selected grid.
         else if ( mResetBtn() )
         {
            mGrids[mSelectedGridIndex]->reset();
         }

         // If the show/hide button has been toggled on, toggle the visible
         // state of the selected grid.
         if ( mHideBtn() )
         {
            mGrids[mSelectedGridIndex]->setVisible(
               ! mGrids[mSelectedGridIndex]->isVisible()
            );

            // Determine if at least one grid is visible now. If there is one
            // such grid, then mGridsVisible should remain true. Otherwise,
            // it should be set to false so that all the grids can be toggled
            // on at once.
            bool grid_visible(false);
            typedef std::vector<GridPtr>::iterator iter_type;
            for ( iter_type g = mGrids.begin(); g != mGrids.end(); ++g )
            {
               if ( (*g)->isVisible() )
               {
                  grid_visible = true;
                  break;
               }
            }

            mGridsVisible = grid_visible;
         }
      }
   }
}

void GridPlugin::focusChanged(ViewerPtr viewer)
{
   ScenePtr scene = viewer->getSceneObj();
   StatusPanelDataPtr status_panel_data =
      scene->getSceneData<StatusPanelData>();

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

      if ( mActivateBtn.isConfigured() )
      {
         status_panel_data->removeControlText(mActivateBtn.toString(),
                                              mActivateText);
      }

      if ( mCycleBtn.isConfigured() )
      {
         status_panel_data->removeControlText(mCycleBtn.toString(),
                                              mCycleText);
      }

      if ( mHideBtn.isConfigured() )
      {
         status_panel_data->removeControlText(mHideBtn.toString(), mHideText);
      }

      if ( mResetBtn.isConfigured() )
      {
         status_panel_data->removeControlText(mResetBtn.toString(),
                                              mResetText);
      }
   }
   else
   {
      if ( mActivateBtn.isConfigured() )
      {
         status_panel_data->addControlText(mActivateBtn.toString(),
                                           mActivateText, 1);
      }

      if ( mCycleBtn.isConfigured() )
      {
         status_panel_data->addControlText(mCycleBtn.toString(), mCycleText,
                                           1);
      }

      if ( mHideBtn.isConfigured() )
      {
         status_panel_data->addControlText(mHideBtn.toString(), mHideText, 1);
      }

      if ( mResetBtn.isConfigured() )
      {
         status_panel_data->addControlText(mResetBtn.toString(), mResetText,
                                           1);
      }
   }
}

void GridPlugin::configure(jccl::ConfigElementPtr elt)
{
   vprASSERT(elt->getID() == getElementType());

   const unsigned int req_cfg_version(1);

   // Check for correct version of plugin configuration.
   if ( elt->getVersion() < req_cfg_version )
   {
      std::ostringstream msg;
      msg << "Configuration of GridPlugin failed.  Required config "
          << "element version is " << req_cfg_version << ", but element '"
          << elt->getName() << "' is version " << elt->getVersion();
      throw PluginException(msg.str(), VRKIT_LOCATION);
   }

   const std::string activate_btn_prop("activate_button_nums");
   const std::string cycle_btn_prop("cycle_button_nums");
   const std::string hide_btn_prop("hide_button_nums");
   const std::string reset_btn_prop("reset_button_nums");
   const std::string grids_prop("grids");

   mActivateBtn.configure(elt->getProperty<std::string>(activate_btn_prop),
                          mWandInterface);
   mCycleBtn.configure(elt->getProperty<std::string>(cycle_btn_prop),
                       mWandInterface);
   mHideBtn.configure(elt->getProperty<std::string>(hide_btn_prop),
                      mWandInterface);
   mResetBtn.configure(elt->getProperty<std::string>(reset_btn_prop),
                       mWandInterface);

   const unsigned int num_grids(elt->getNum(grids_prop));

   for ( unsigned int i = 0; i < num_grids; ++i )
   {
      try
      {
         mGrids.push_back(
            Grid::create()->init(
               elt->getProperty<jccl::ConfigElementPtr>(grids_prop, i)
            )
         );
      }
      catch (Exception& ex)
      {
         std::cerr << "Failed to configure grid #" << i << ":\n" << ex.what()
                   << std::endl;
      }
   }
}

}
