// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_STATUS_PANEL_PLUGIN_H_
#define _INF_STATUS_PANEL_PLUGIN_H_

#include <IOV/Plugin.h>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <OpenSG/OSGTransform.h>

#include <vpr/vpr.h>
#include <vpr/Util/GUID.h>


namespace inf
{

class StatusPanel;

class StatusPanelPlugin;
typedef boost::shared_ptr<StatusPanelPlugin> StatusPanelPluginPtr;
typedef boost::weak_ptr<StatusPanelPlugin> StatusPanelPluginWeakPtr;

class IOV_CLASS_API StatusPanelPlugin
   : public inf::Plugin, public boost::enable_shared_from_this<StatusPanelPlugin>
{
protected:
   StatusPanelPlugin();

public:
   static inf::PluginPtr create();

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

   virtual void destroy();
};

}

#endif
