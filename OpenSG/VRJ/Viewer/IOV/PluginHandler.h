#ifndef _INF_PLUGIN_HANDLER_H_
#define _INF_PLUGIN_HANDLER_H_

#include <OpenSG/VRJ/Viewer/IOV/Config.h>

#include <iostream>
#include <string>
#include <vpr/vpr.h>

#include <OpenSG/VRJ/Viewer/IOV/Plugin.h>
#include <OpenSG/VRJ/Viewer/IOV/PluginCreator.h>


namespace inf
{

class IOV_CLASS_API PluginHandler
{
public:
   static const std::string GET_VERSION_FUNC;
   static const std::string GET_CREATOR_FUNC;
};

class IOV_CLASS_API VersionCheckCallable
{
public:
   VersionCheckCallable()
   {
   }

   bool operator()(void* func)
   {
      void (*version_func)(vpr::Uint32&, vpr::Uint32&);
      version_func = (void (*)(vpr::Uint32&, vpr::Uint32&)) func;

      unsigned int major_ver;
      unsigned int minor_ver;
      (*version_func)(major_ver, minor_ver);

      // TODO: Now do something with the version info that we got back ...

      return true;
   }

private:
   static const vpr::Uint32 sPluginMajor;
   static const vpr::Uint32 sPluginMinor;
};

template<class T>
struct PluginCreateCallable
{
   PluginCreateCallable(T handler)
      : mHandler(handler)
   {
   }

   /**
    * Creates an instance of the loaded plug-in and adds it to the handler
    * given in the constructor.
    */
   bool operator()(void* func)
   {
      inf::PluginCreator* (*get_creator_func)();

      get_creator_func = (inf::PluginCreator* (*)()) func;
      inf::PluginCreator* creator = (*get_creator_func)();
      inf::PluginPtr plugin = creator->createPlugin();

      if ( NULL == plugin.get() )
      {
         std::cerr << "Plug-in creation failed." << std::endl;
      }
      else
      {
         mHandler->addPlugin(plugin);
      }

      return true;
   }

   T mHandler;
};

}


#endif
