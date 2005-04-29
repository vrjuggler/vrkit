#ifndef _INF_POINT_GRAB_PLUGIN_PTR_H_
#define _INF_POINT_GRAB_PLUGIN_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class PointGrabPlugin;
   typedef boost::shared_ptr<PointGrabPlugin> PointGrabPluginPtr;
   typedef boost::weak_ptr<PointGrabPlugin> PointGrabPluginWeakPtr;
}

#endif
