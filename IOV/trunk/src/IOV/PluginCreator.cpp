// Copyright (C) Infiscape Corporation 2005

#include <sstream>

#include <IOV/PluginCreator.h>


namespace inf
{

PluginCreator::PluginCreator(boost::function<inf::PluginPtr ()> creator,
                             const std::string& pluginName,
                             const vpr::Uint32 pluginMajorVer,
                             const vpr::Uint32 pluginMinorVer,
                             const vpr::Uint32 pluginPatchVer)
   : mCreator(creator)
   , mPluginName(pluginName)
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
