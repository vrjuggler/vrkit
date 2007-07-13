// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_IOV_PLUGIN_REGISTRY_H_
#define _INF_IOV_PLUGIN_REGISTRY_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class PluginRegistry;
   typedef boost::shared_ptr<PluginRegistry> PluginRegistryPtr;
   typedef boost::weak_ptr<PluginRegistry> PluginRegistryWeakPtr;   
}

#endif
