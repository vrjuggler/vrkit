// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_VOLUME_DRAWING_PLUGIN_H_
#define _INF_VOLUME_DRAWING_PLUGIN_H_

#include <IOV/Plugin/PluginConfig.h>

#include <boost/enable_shared_from_this.hpp>

#include <IOV/Plugin.h>
#include <IOV/ViewerPtr.h>


namespace inf
{

class VolumeDrawingPlugin
   : public inf::Plugin
   , public boost::enable_shared_from_this<VolumeDrawingPlugin>
{
protected:
   VolumeDrawingPlugin();

public:
   static inf::PluginPtr create()
   {
      return inf::PluginPtr(new VolumeDrawingPlugin());
   }

   virtual ~VolumeDrawingPlugin()
   {
      /* Do nothing. */ ;
   }

   virtual std::string getDescription()
   {
      return std::string("Volume Drawing");
   }

   /**
    * Initializes this plug-in. Currently, there is nothing to initialize.
    *
    * @param viewer The VR Juggler application object.
    *
    * @return A pointer to this plug-in is returned.
    */
   virtual PluginPtr init(inf::ViewerPtr viewer);

   /**
    * Activates OpenSG volume drawing for the newly opened context.
    *
    * @post The context-specific render action for this context will render
    *       node bounding volumes.
    *
    * @param viewer The VR Juggler application object.
    */
   virtual void contextInit(inf::ViewerPtr viewer);

   /**
    * Does nothing as this plug-in does not have any state.
    *
    * @param viewer The VR Juggler application object.
    */
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
};

}


#endif /* _INF_VOLUME_DRAWING_PLUGIN_H_ */
