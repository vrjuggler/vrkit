#include <sstream>

#include <OpenSG/VRJ/Viewer/IOV/PluginCreator.h>


namespace inf
{

PluginCreator::PluginCreator(const std::string& pluginName,
                             const vpr::Uint32 pluginMajorVer,
                             const vpr::Uint32 pluginMinorVer,
                             const vpr::Uint32 pluginPatchVer)
   : mPluginName(pluginName)
   , mPluginMajorVer(pluginMajorVer)
   , mPluginMinorVer(pluginMinorVer)
   , mPluginPatchVer(pluginPatchVer)
{
   std::stringstream str_stream;
   str_stream << mPluginMajorVer << "." << mPluginMinorVer << "."
              << mPluginPatchVer;
   mPluginVersionStr = str_stream.str();
}

PluginCreator::~PluginCreator()
{
}

}
