#include <vpr/vpr.h>
#include <vpr/DynLoad/LibraryLoader.h>

#include <OpenSG/VRJ/Viewer/User.h>
#include <OpenSG/VRJ/Viewer/Plugin.h>
#include <OpenSG/VRJ/Viewer/Viewer.h>


namespace
{

struct VersionCheckCallable
{
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

   static const vpr::Uint32 sPluginMajor;
   static const vpr::Uint32 sPluginMinor;
};

const vpr::Uint32 VersionCheckCallable::sPluginMajor(INF_PLUGIN_API_MAJOR);
const vpr::Uint32 VersionCheckCallable::sPluginMinor(INF_PLUGIN_API_MINOR);

struct PluginCreateCallable
{
   PluginCreateCallable(inf::ViewerPtr viewer)
      : mViewer(viewer)
   {
   }

   bool operator()(void* func)
   {
      inf::PluginPtr (*create_func)();

      create_func = (inf::PluginPtr (*)()) func;
      inf::PluginPtr plugin = (*create_func)();

      if ( NULL == plugin.get() )
      {
         std::cerr << "Plug-in creation failed." << std::endl;
      }
      else
      {
         std::cout << "Initializing the plug-in ..." << std::endl;
         // Initialize the newly loaded plug-in.
         plugin->init(mViewer);

         mViewer->addPlugin(plugin);
      }

      return true;
   }

   inf::ViewerPtr mViewer;
};

}

namespace inf
{

void Viewer::init()
{
   vrj::OpenSGApp::init();

   // Create an initialize the user
   mUser = User::create();
   mUser->init();

   // Create and initialize the base scene object
   mScene = Scene::create();
   mScene->init();

   // XXX: This bit is hard-coded for now while I get it working.
   std::vector<std::string> search_path(1);
   search_path[0] = "plugins";
   vpr::LibraryPtr dso = vpr::LibraryLoader::findDSO("SimpleNavPlugin",
                                                     search_path);

   if ( dso.get() != NULL )
   {
      const std::string get_version_func("getPluginInterfaceVersion");
      const std::string create_func("create");

      VersionCheckCallable version_functor;

      vpr::ReturnStatus version_status =
         vpr::LibraryLoader::findEntryPoint(dso, get_version_func,
                                            version_functor);

      if ( ! version_status.success() )
      {
         std::cerr << "Version mismatch!" << std::endl;
      }
      else
      {
         PluginCreateCallable create_functor(shared_from_this());

         vpr::ReturnStatus create_status;
         create_status = vpr::LibraryLoader::findEntryPoint(dso, create_func,
                                                            create_functor);

         if ( create_status.success() )
         {
            mLoadedDsos.push_back(dso);
         }
      }
   }
   else
   {
      std::cerr << "Failed to load plug-in!" << std::endl;
   }
}

void Viewer::preFrame()
{
   std::vector<inf::PluginPtr>::iterator i;
   for ( i = mPlugins.begin(); i != mPlugins.end(); ++i )
   {
      (*i)->update(shared_from_this());
   }

   // Update the user (and navigation)
   getUser()->update(shared_from_this());
}

}
