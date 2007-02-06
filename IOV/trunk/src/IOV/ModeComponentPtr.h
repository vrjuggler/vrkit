// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_MODE_COMPONENT_PTR_H_
#define _INF_MODE_COMPONENT_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class ModeComponent;
   typedef boost::shared_ptr<ModeComponent> ModeComponentPtr;
   typedef boost::weak_ptr<ModeComponent> ModeComponentWeakPtr;
}

#endif
