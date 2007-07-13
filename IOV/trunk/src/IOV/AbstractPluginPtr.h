// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_ABSTRACT_PLUGIN_PTR_H_
#define _INF_ABSTRACT_PLUGIN_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class AbstractPlugin;
   typedef boost::shared_ptr<AbstractPlugin> AbstractPluginPtr;
   typedef boost::weak_ptr<AbstractPlugin> AbstractPluginWeakPtr;
}

#endif
