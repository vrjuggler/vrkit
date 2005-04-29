#include <OpenSG/VRJ/Viewer/IOV/PluginHandler.h>


namespace inf
{

const std::string PluginHandler::GET_VERSION_FUNC("getPluginInterfaceVersion");
const std::string PluginHandler::CREATE_FUNC("create");

const vpr::Uint32 VersionCheckCallable::sPluginMajor(INF_PLUGIN_API_MAJOR);
const vpr::Uint32 VersionCheckCallable::sPluginMinor(INF_PLUGIN_API_MINOR);

}
