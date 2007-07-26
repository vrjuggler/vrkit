// Copyright (C) Infiscape Corporation 2005-2007

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

   /**
    * Invokes the global scope delete operator.  This is required for proper
    * releasing of memory in DLLs on Win32.
    */
   void operator delete(void* p)
   {
      ::operator delete(p);
   }

protected:
   StatusPanel			mStatusPanel;        /**< The status panel we are using. */
   StatusPanelViewOriginal	mStatusPanelView;    /**< The status panel view that we are using. */
   OSG::TransformNodePtr	mPanelXformNode;     /**< Root node of panel. */
   OSG::SwitchNodePtr		mPanelVisSwitchNode; /**< Switch Node for hiding panel geometry. */

   boost::signals::connection mOutputConn;

   virtual void destroy();
};

}


#endif
