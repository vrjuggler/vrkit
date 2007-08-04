// Copyright (C) Infiscape Corporation 2005-2007

#include <algorithm>
#include <boost/bind.hpp>
#include <boost/assign/list_of.hpp>

#include <gmtl/Generate.h>
#include <gmtl/External/OpenSGConvert.h>

#include <vpr/vpr.h>
#include <jccl/Config/ConfigElement.h>

#include <IOV/PluginCreator.h>
#include <IOV/StatusPanelData.h>
#include <IOV/Status.h>
#include <IOV/User.h>
#include <IOV/Viewer.h>
#include <IOV/WandInterface.h>
#include <IOV/Version.h>
#include <IOV/Plugin/Info.h>

#include "Grid.h"
#include "GridPlugin.h"


using namespace boost::assign;

static const inf::plugin::Info sInfo(
   "com.infiscape", "GridPlugin",
   list_of(IOV_VERSION_MAJOR)(IOV_VERSION_MINOR)(IOV_VERSION_PATCH)
);
static inf::PluginCreator<inf::Plugin> sPluginCreator(
   boost::bind(&inf::GridPlugin::create, sInfo)
);

extern "C"
{

/** @name Plug-in Entry Points */
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
   return &sPluginCreator;
}
//@}
}

namespace inf
{

GridPlugin::GridPlugin(const inf::plugin::Info& info)
   : Plugin(info)
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

PluginPtr GridPlugin::init(inf::ViewerPtr viewer)
{
   mWandInterface = viewer->getUser()->getInterfaceTrader().getWandInterface();

   jccl::ConfigElementPtr cfg_elt =
      viewer->getConfiguration().getConfigElement(getElementType());

   if ( cfg_elt )
   {
      configure(cfg_elt);
   }

   std::vector<inf::GridPtr>::iterator g;
   for ( g = mGrids.begin(); g != mGrids.end(); ++g )
   {
      OSG::GroupNodePtr decorator_root =
         viewer->getSceneObj()->getDecoratorRoot();
      OSG::beginEditCP(decorator_root.node(), OSG::Node::ChildrenFieldMask);
         decorator_root.node()->addChild((*g)->getRoot());
      OSG::endEditCP(decorator_root.node(), OSG::Node::ChildrenFieldMask);
      (*g)->setVisible(mGridsVisible);
   }

   return shared_from_this();
}

void GridPlugin::update(inf::ViewerPtr viewer)
{
   if ( isFocused() )
   {
      // Change the visiblity of all the grids of the activate/deactivate
      // button is toggled on.
      if ( mActivateBtn() )
      {
         mGridsVisible = ! mGridsVisible;
         std::vector<inf::GridPtr>::iterator g;
         for ( g = mGrids.begin(); g != mGrids.end(); ++g )
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

            IOV_STATUS << "Grid #" << mSelectedGridIndex
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

            inf::GridPtr grid = mGrids[mSelectedGridIndex];
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
            std::vector<inf::GridPtr>::iterator g;
            for ( g = mGrids.begin(); g != mGrids.end(); ++g )
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

void GridPlugin::focusChanged(inf::ViewerPtr viewer)
{
   inf::ScenePtr scene = viewer->getSceneObj();
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
         status_panel_data->mRemoveControlText(mActivateBtn.toString(),
                                               mActivateText);
      }

      if ( mCycleBtn.isConfigured() )
      {
         status_panel_data->mRemoveControlText(mCycleBtn.toString(),
                                               mCycleText);
      }

      if ( mHideBtn.isConfigured() )
      {
         status_panel_data->mRemoveControlText(mHideBtn.toString(),
                                               mHideText);
      }

      if ( mResetBtn.isConfigured() )
      {
         status_panel_data->mRemoveControlText(mResetBtn.toString(),
                                               mResetText);
      }
   }
   else
   {
      if ( mActivateBtn.isConfigured() )
      {
         bool has(false);
         status_panel_data->mHasControlText(mActivateBtn.toString(),
                                            mActivateText, has);

         if ( ! has )
         {
            status_panel_data->mAddControlText(mActivateBtn.toString(),
                                               mActivateText, 1);
         }
      }

      if ( mCycleBtn.isConfigured() )
      {
         bool has(false);
         status_panel_data->mHasControlText(mCycleBtn.toString(), mCycleText,
                                            has);

         if ( ! has )
         {
            status_panel_data->mAddControlText(mCycleBtn.toString(),
                                               mCycleText, 1);
         }
      }

      if ( mHideBtn.isConfigured() )
      {
         bool has(false);
         status_panel_data->mHasControlText(mHideBtn.toString(), mHideText,
                                            has);

         if ( ! has )
         {
            status_panel_data->mAddControlText(mHideBtn.toString(), mHideText,
                                               1);
         }
      }

      if ( mResetBtn.isConfigured() )
      {
         bool has(false);
         status_panel_data->mHasControlText(mResetBtn.toString(), mResetText,
                                            has);

         if ( ! has )
         {
            status_panel_data->mAddControlText(mResetBtn.toString(),
                                               mResetText, 1);
         }
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
      throw PluginException(msg.str(), IOV_LOCATION);
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
            inf::Grid::create()->init(
               elt->getProperty<jccl::ConfigElementPtr>(grids_prop, i)
            )
         );
      }
      catch (inf::Exception& ex)
      {
         std::cerr << "Failed to configure grid #" << i << ":\n" << ex.what()
                   << std::endl;
      }
   }
}

}
