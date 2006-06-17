// Copyright (C) Infiscape Corporation 2005-2006

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
public:
   static inf::PluginPtr create();
   
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

   /**
    * Invokes the global scope delete operator.  This is required for proper
    * releasing of memory in DLLs on Win32.
    */
   void operator delete(void* p)
   {
      ::operator delete(p);
   }
   
protected:
   /**
    * Deletes this object.  This is an implementation of the pure virtual
    * inf::Plugin::destroy() method.
    */
   virtual void destroy()
   {
      delete this;
   }
   
   ModelLoaderPlugin()
   {
      /* Do nothing. */ ;
   }

}; // ModelLoaderPlugin
   
}  // namespace inf

#endif
