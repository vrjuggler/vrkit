#ifndef DEVICES_PTR_H_
#define DEVICES_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
namespace inf
{
   class Devices;
   typedef boost::shared_ptr<Devices> DevicesPtr;
   typedef boost::weak_ptr<Devices> DevicesWeakPtr;
}

#endif

