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

#ifndef _STATUS_PANEL_PLUGIN_H_
#define _STATUS_PANEL_PLUGIN_H_

#include <IOV/Plugin.h>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <OpenSG/OSGTransform.h>

#include <vpr/vpr.h>
#include <vpr/Util/GUID.h>

#include <IOV/SceneData.h>


namespace inf
{

class StatusPanel;

class StatusPanelPlugin;
typedef boost::shared_ptr<StatusPanelPlugin> StatusPanelPluginPtr;
typedef boost::weak_ptr<StatusPanelPlugin> StatusPanelPluginWeakPtr;

class IOV_CLASS_API StatusPanelPlugin
   : public inf::Plugin, public boost::enable_shared_from_this<StatusPanelPlugin>
{
public:
   static inf::PluginPtr create();

   virtual ~StatusPanelPlugin();

   virtual std::string getDescription();

   /** Initialize the plugin. */
   virtual void init(inf::ViewerPtr viewer);

   virtual void updateState(inf::ViewerPtr viewer);

   virtual void run(inf::ViewerPtr viewer);

   /** Get status panel for direct usage. */
   StatusPanel& getPanel();

   /**
    * Invokes the global scope delete operator.  This is required for proper
    * releasing of memory in DLLs on Win32.
    */
   void operator delete(void* p)
   {
      ::operator delete(p);
   }

protected:
   StatusPanel*            mStatusPanel;     /**< The status panel we are using. */
   OSG::TransformNodePtr   mPanelXformNode;  /**< Root node of panel. */

protected:
   StatusPanelPlugin();

   virtual void destroy();
};


class StatusPanelPluginData;
typedef boost::shared_ptr<StatusPanelPluginData> StatusPanelPluginDataPtr;

class IOV_CLASS_API StatusPanelPluginData : public inf::SceneData
{
public:
   static const vpr::GUID type_guid;

   static StatusPanelPluginDataPtr create()
   {
      return StatusPanelPluginDataPtr(new StatusPanelPluginData);
   }

   StatusPanelPluginData();

   virtual ~StatusPanelPluginData();

public:
   StatusPanelPluginPtr    mStatusPanelPlugin;  /**< Pointer to the plugin (if registered). */
};

}

#endif
