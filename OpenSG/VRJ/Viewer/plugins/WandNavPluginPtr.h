#ifndef WAND_NAV_PLUGIN_PTR_H_
#define WAND_NAV_PLUGIN_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class WandNavPlugin;
   typedef boost::shared_ptr<WandNavPlugin> WandNavPluginPtr;
   typedef boost::weak_ptr<WandNavPlugin> WandNavPluginWeakPtr;
}

#endif

