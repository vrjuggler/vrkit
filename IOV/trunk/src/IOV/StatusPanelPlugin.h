#ifndef _STATUS_PANEL_PLUGIN_H_
#define _STATUS_PANEL_PLUGIN_H_

#include <IOV/Plugin.h>
#include <IOV/StatusPanel.h>
#include <IOV/SceneData.h>

#include <OpenSG/OSGTransform.h>

#include <vpr/vpr.h>
#include <vpr/Util/GUID.h>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>


namespace inf
{
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

protected:
   StatusPanel             mStatusPanel;     /**< The status panel we are using. */
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
