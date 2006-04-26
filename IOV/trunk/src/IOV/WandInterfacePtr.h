// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_WAND_INTERFACE_PTR_H_
#define _INF_WAND_INTERFACE_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
namespace inf
{
   class WandInterface;
   typedef boost::shared_ptr<WandInterface> WandInterfacePtr;
   typedef boost::weak_ptr<WandInterface> WandInterfaceWeakPtr;   
}

#endif

