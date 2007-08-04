// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_SIGNAL_CONTAINER_BASE_PTR_H_
#define _INF_SIGNAL_CONTAINER_BASE_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class SignalContainerBase;
   typedef boost::shared_ptr<SignalContainerBase> SignalContainerBasePtr;
   typedef boost::weak_ptr<SignalContainerBase> SignalContainerBaseWeakPtr;
}

#endif /*_INF_SIGNAL_CONTAINER_BASE_PTR_H_*/
