// vrkit is (C) Copyright 2005-2011
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

#ifndef _VRKIT_GRID_PLUGIN_H_
#define _VRKIT_GRID_PLUGIN_H_

#include <vrkit/plugin/Config.h>

#include <string>
#include <vector>
#include <boost/enable_shared_from_this.hpp>

#include <jccl/Config/ConfigElementPtr.h>

#include <vrkit/WandInterfacePtr.h>
#include <vrkit/ViewerPtr.h>
#include <vrkit/viewer/Plugin.h>
#include <vrkit/util/DigitalCommand.h>

#include "GridPtr.h"


namespace vrkit
{

class GridPlugin
   : public viewer::Plugin
   , public boost::enable_shared_from_this<GridPlugin>
{
protected:
   GridPlugin(const plugin::Info& info);

public:
   static viewer::PluginPtr create(const plugin::Info& info)
   {
      return viewer::PluginPtr(new GridPlugin(info));
   }

   virtual ~GridPlugin()
   {
      /* Do nothing. */ ;
   }

   virtual std::string getDescription()
   {
      return std::string("Grid");
   }

   virtual viewer::PluginPtr init(ViewerPtr viewer);

   /**
    * Updates the state of the grids based on input from the wand interface.
    *
    * @post The state of each grid is updated to reflect the digtial and/or
    *       analog input from the user via the wand interface.
    *
    * @param viewer The VR Juggler application object.
    */
   virtual void update(ViewerPtr viewer);

protected:
   /**
    * Updates the state of the status panel (if it is in use) based on the
    * new focused state of this plug-in.
    *
    * @post If this plug-in has lost focus, \c mSelectedGridIndex will be -1.
    *       This means that no grid will be selected the next time that this
    *       plug-in is activated. If one or more grids are visible when focus
    *       is lost, they remain visible, but none are selected.
    *
    * @param viewer The VR Juggler application object.
    */
   void focusChanged(ViewerPtr viewer);

private:
   static std::string getElementType()
   {
      return std::string("vrkit_grid_plugin");
   }

   /**
    * Configures this plug-in.
    *
    * @pre The type of \p elt matches the value returned by getElementType().
    *
    * @param elt The config element for this plug-in.
    *
    * @throw vrkit::Exception
    *           Thrown if the given config element contains bad data, is out
    *           of date, or is otherwise unusable.
    */
   void configure(jccl::ConfigElementPtr elt);

   WandInterfacePtr mWandInterface;

   /** @name Grid State */
   //@{
   std::vector<GridPtr> mGrids;
   int mSelectedGridIndex;
   bool mGridsVisible;
   //@}

   /** @name Grid Sliding */
   //@{
   int   mAnalogNum;    /**< Wand interface analog index for sliding */
   float mForwardVal;   /**< "Forward" analog value (0.0 or 1.0) */
   //@}

   /** @name Digital Buttons */
   //@{
   util::DigitalCommand mActivateBtn;    /**< Activate/deactivate all button */
   util::DigitalCommand mCycleBtn;       /**< Cycle grid selection button */
   util::DigitalCommand mHideBtn;        /**< Show/hide grid button */
   util::DigitalCommand mResetBtn;       /**< Reset grid position button */
   //@}

   /** @name Digital Button Command Descriptions */
   //@{
   const std::string mActivateText;     /**< Activation command description */
   const std::string mCycleText;        /**< Grid cycle command description */
   const std::string mHideText;         /**< Grid hide command description */
   const std::string mResetText;        /**< Grid reset command description */
   //@}
};

}


#endif /* _INF_GRID_PLUGIN_H_ */
