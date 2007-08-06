// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_MODEL_LOADER_PLUGIN_H_
#define _INF_MODEL_LOADER_PLUGIN_H_

#include <string>
#include <boost/enable_shared_from_this.hpp>

#include <IOV/Plugin/PluginConfig.h>
#include <IOV/Plugin.h>

namespace inf
{

class ModelLoaderPlugin
   : public inf::Plugin
   , public boost::enable_shared_from_this<ModelLoaderPlugin>
{
protected:
   ModelLoaderPlugin(const inf::plugin::Info& info)
      : Plugin(info)
   {
      /* Do nothing. */ ;
   }

public:
   static inf::PluginPtr create(const inf::plugin::Info& info);

   virtual ~ModelLoaderPlugin()
   {
      /* Do nothing. */ ;
   }

   virtual std::string getDescription();

   /** Initialize and configure the plugin.
    *
    */
   virtual PluginPtr init(inf::ViewerPtr viewer);

   virtual void update(inf::ViewerPtr viewer);
}; // ModelLoaderPlugin

}  // namespace inf

#endif
