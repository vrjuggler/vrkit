// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_SIGNAL_REPOSITORY_PTR_H_
#define _INF_SIGNAL_REPOSITORY_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class SignalRepository;
   typedef boost::shared_ptr<SignalRepository> SignalRepositoryPtr;
   typedef boost::weak_ptr<SignalRepository> SignalRepositoryWeakPtr;   
}

#endif /*_INF_SIGNAL_REPOSITORY_PTR_H_*/
