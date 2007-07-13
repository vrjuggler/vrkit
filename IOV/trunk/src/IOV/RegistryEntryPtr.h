// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_IOV_REGISTRY_ENTRY_H_
#define _INF_IOV_REGISTRY_ENTRY_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class RegistryEntry;
   typedef boost::shared_ptr<RegistryEntry> RegistryEntryPtr;
   typedef boost::weak_ptr<RegistryEntry> RegistryEntryWeakPtr;   
}

#endif
