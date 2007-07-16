// Copyright (C) Infiscape Corporation 2005-2007

#include <OpenSG/OSGConfig.h>

#include <boost/bind.hpp>
#include <boost/assign/list_of.hpp>

#include <IOV/PluginCreator.h>
#include <IOV/Viewer.h>
#include <IOV/Version.h>
#include <IOV/Plugin/Info.h>

#include "VolumeDrawingPlugin.h"


using namespace boost::assign;

static const inf::plugin::Info sInfo(
   "com.infiscape", "VolumeDrawingPlugin",
   list_of(IOV_VERSION_MAJOR)(IOV_VERSION_MINOR)(IOV_VERSION_PATCH)
);
static inf::PluginCreator<inf::Plugin> sPluginCreator(
   boost::bind(&inf::VolumeDrawingPlugin::create, sInfo)
);

extern "C"
{

/** @name Plug-in Entry Points */
//@{
IOV_PLUGIN_API(const inf::plugin::Info*) getPluginInfo()
{
   return &sInfo;
}

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

VolumeDrawingPlugin::VolumeDrawingPlugin(const inf::plugin::Info& info)
   : inf::Plugin(info)
{
   /* Do nothing. */ ;
}

PluginPtr VolumeDrawingPlugin::init(inf::ViewerPtr)
{
   return shared_from_this();
}

void VolumeDrawingPlugin::contextInit(inf::ViewerPtr viewer)
{
   viewer->getRenderAction()->setVolumeDrawing(isFocused());
}

void VolumeDrawingPlugin::update(inf::ViewerPtr)
{
   /* Do nothing. */ ;
}

}
