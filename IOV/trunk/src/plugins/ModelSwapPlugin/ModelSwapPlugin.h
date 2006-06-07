// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_MODEL_SWAP_PLUGIN_H_
#define _INF_MODEL_SWAP_PLUGIN_H_

#include <string>
#include <boost/enable_shared_from_this.hpp>

#include <OpenSG/OSGRefPtr.h>
#include <OpenSG/OSGSwitch.h>

#include <IOV/Plugin/PluginConfig.h>
#include <IOV/Plugin.h>
#include <IOV/WandInterfacePtr.h>
#include <IOV/Util/DigitalCommand.h>

namespace inf
{

class ModelSwapPlugin
   : public inf::Plugin
   , public boost::enable_shared_from_this<ModelSwapPlugin>
{
public:
   static inf::PluginPtr create();
   
   virtual ~ModelSwapPlugin()
   {
      /* Do nothing. */ ;
   }
   
   virtual std::string getDescription();
   
   /** Initialize and configure the plugin.
    * 
    */
   virtual PluginPtr init(inf::ViewerPtr viewer);
   
   virtual void updateState(inf::ViewerPtr viewer);

   virtual void run(inf::ViewerPtr viewer);
   
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
   
   ModelSwapPlugin()
   {
      /* Do nothing. */ ;
   }

protected:
   WandInterfacePtr        mWandInterface;      /**< Ptr to the wand interface to use. */
   inf::DigitalCommand     mSwapButton;       /**< Ptr to the digital button. */
   OSG::NodeRefPtr            mSwitchNode;
   OSG::SwitchRefPtr          mSwitchCore;

}; // ModelSwapPlugin
   
}  // namespace inf

#endif
