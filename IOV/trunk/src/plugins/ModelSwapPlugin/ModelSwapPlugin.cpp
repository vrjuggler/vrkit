#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#endif

#include <IOV/Viewer.h>
#include <IOV/WandInterface.h>
#include <IOV/Util/DigitalCommand.h>
#include <IOV/PluginCreator.h>

#include "ModelSwapPlugin.h"


static inf::PluginCreator<inf::Plugin> sPluginCreator(
   &inf::ModelSwapPlugin::create, "Model Swap Plug-in"
);

extern "C"
{

/** @name Plug-in Entry Points */
//@{
IOV_PLUGIN_API(void) getPluginInterfaceVersion(vpr::Uint32& majorVer,
                                               vpr::Uint32& minorVer)
{
   majorVer = INF_PLUGIN_API_MAJOR;
   minorVer = INF_PLUGIN_API_MINOR;
}

IOV_PLUGIN_API(inf::PluginCreatorBase*) getCreator()
{
   return &sPluginCreator;
}
//@}

}


namespace inf
{
   PluginPtr ModelSwapPlugin::create()
   {
      return PluginPtr(new ModelSwapPlugin());
   }
   
   std::string ModelSwapPlugin::getDescription()
   {
      return std::string("Model Swap Plugin");
   }
   
   PluginPtr ModelSwapPlugin::init(inf::ViewerPtr viewer)
   {
      // TODO: fill in initialization code
      return shared_from_this(); 
   }
   
   void ModelSwapPlugin::updateState(inf::ViewerPtr viewer)
   {
      // TODO: fill in update state code
   }
   
   void ModelSwapPlugin::run(inf::ViewerPtr viewer)
   {
      // TODO: fill in run code
   }
   
   
}  // namespace inf
