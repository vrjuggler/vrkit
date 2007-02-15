// Copyright (C) Infiscape Corporation 2005-2006

#ifndef PLUGIN_PTR_H_
#define PLUGIN_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
namespace inf
{
   class Plugin;
   typedef boost::shared_ptr<Plugin> PluginPtr;
   typedef boost::weak_ptr<Plugin> PluginWeakPtr;
}

#endif

