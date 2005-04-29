#ifndef _INF_MODE_SWITCH_PLUGIN_PTR_H_
#define _INF_MODE_SWITCH_PLUGIN_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class ModeSwitchPlugin;
   typedef boost::shared_ptr<ModeSwitchPlugin> ModeSwitchPluginPtr;
   typedef boost::weak_ptr<ModeSwitchPlugin> ModeSwitchPluginWeakPtr;
}

#endif
