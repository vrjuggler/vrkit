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

#ifndef _INF_STATUS_PANEL_PLUGIN_H_
#define _INF_STATUS_PANEL_PLUGIN_H_

#include <IOV/Plugin.h>
#include <IOV/Plugin/PluginConfig.h>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/signals/connection.hpp>

#include <OpenSG/OSGTransform.h>
#include <OpenSG/OSGSwitch.h>

#include <vpr/vpr.h>
#include <vpr/Util/GUID.h>

#include "StatusPanel.h"
#include "StatusPanelViewOriginal.h"


namespace inf
{

class StatusPanelPlugin
   : public inf::Plugin
   , public boost::enable_shared_from_this<StatusPanelPlugin>
{
protected:
   StatusPanelPlugin(const inf::plugin::Info& info);

public:
   static inf::PluginPtr create(const inf::plugin::Info& info);

   virtual ~StatusPanelPlugin();

   virtual std::string getDescription();

   /**
    * Initializes this plug-in.
    *
    * @return This object is returned as a shared pointer.
    */
   virtual inf::PluginPtr init(inf::ViewerPtr viewer);

   virtual void update(inf::ViewerPtr viewer);

   /** Get status panel for direct usage. */
   StatusPanel& getPanel();

   /**
    * Sets the visibility of the StatusPanel scenegraph.
    *
    * @post The StatusPanel scenegraph reflects the desired visibility
    * 	    state.
    *
    * @param visible The new desired visibility state.
    */
   void setVisibility(bool visible);

protected:
   StatusPanel			mStatusPanel;        /**< The status panel we are using. */
   StatusPanelViewOriginal	mStatusPanelView;    /**< The status panel view that we are using. */
   OSG::TransformNodePtr	mPanelXformNode;     /**< Root node of panel. */
   OSG::SwitchNodePtr		mPanelVisSwitchNode; /**< Switch Node for hiding panel geometry. */

   boost::signals::connection mOutputConn;
   boost::signals::connection mVisConn;		/**< Visibility connection object. */
};

}


#endif
