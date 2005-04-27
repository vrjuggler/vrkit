#ifndef SIMPLE_NAV_PLUGIN_PTR_H_
#define SIMPLE_NAV_PLUGIN_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class SimpleNavPlugin;
   typedef boost::shared_ptr<SimpleNavPlugin> SimpleNavPluginPtr;
   typedef boost::weak_ptr<SimpleNavPlugin> SimpleNavPluginWeakPtr;
}

#endif

