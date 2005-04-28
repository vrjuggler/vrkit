#ifndef _CENTER_POINT_GRAB_PLUGIN_PTR_H_
#define _CENTER_POINT_GRAB_PLUGIN_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class CenterPointGrabPlugin;
   typedef boost::shared_ptr<CenterPointGrabPlugin> CenterPointGrabPluginPtr;
   typedef boost::weak_ptr<CenterPointGrabPlugin> CenterPointGrabPluginWeakPtr;
}

#endif
