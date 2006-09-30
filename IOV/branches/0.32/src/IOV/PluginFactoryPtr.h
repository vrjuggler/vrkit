// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_IOV_PLUGIN_FACTORY_H_
#define _INF_IOV_PLUGIN_FACTORY_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class PluginFactory;
   typedef boost::shared_ptr<PluginFactory> PluginFactoryPtr;
   typedef boost::weak_ptr<PluginFactory> PluginFactoryWeakPtr;   
}

#endif
