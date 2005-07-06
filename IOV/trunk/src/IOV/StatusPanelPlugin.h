#ifndef _STATUS_PANEL_PLUGIN_H_
#define _STATUS_PANEL_PLUGIN_H_

#include <IOV/Plugin.h>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <IOV/StatusPanel.h>
#include <OpenSG/OSGTransform.h>

namespace inf
{
class StatusPanelPlugin;
typedef boost::shared_ptr<StatusPanelPlugin> StatusPanelPluginPtr;
typedef boost::weak_ptr<StatusPanelPlugin> StatusPanelPluginWeakPtr;

class StatusPanelPlugin
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

protected:
   StatusPanel             mStatusPanel;     /**< The status panel we are using. */
   OSG::TransformNodePtr   mPanelXformNode;  /**< Root node of panel. */

protected:
   StatusPanelPlugin();

   virtual void destroy();
};


}

#endif
