// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_SIGNAL_CONTAINER_BASE_H_
#define _INF_SIGNAL_CONTAINER_BASE_H_

#include <IOV/Config.h>

#include <boost/noncopyable.hpp>

#include <IOV/SignalContainerBasePtr.h>


namespace inf
{

/**
 * The base class for all signal containers that can be registered with the
 * signal repository. For all intents and purposes, this is an abstract base
 * class, although it does not define any pure virtual methods. Instead, it
 * has a virtual destructor to ensure that this class has a vtable and can be
 * handled polymorphically.
 *
 * inf::SignalRepository
 *
 * @since 0.34
 */
class IOV_CLASS_API SignalContainerBase : private boost::noncopyable
{
protected:
   SignalContainerBase();

public:
   virtual ~SignalContainerBase();
};

}


#endif /* _INF_SIGNAL_CONTAINER_BASE_H_ */
