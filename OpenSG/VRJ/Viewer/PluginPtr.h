#ifndef PLUGIN_PTR_H_
#define PLUGIN_PTR_H_

#include <boost/shared_ptr.hpp>
namespace inf
{
   class Plugin;
   typedef boost::shared_ptr<Plugin> PluginPtr;
}

#endif

