// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_WIDGET_SPHERE_BUTTON_PTR_H_
#define _INF_WIDGET_SPHERE_BUTTON_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class SphereButton;
   typedef boost::shared_ptr<SphereButton> SphereButtonPtr;
   typedef boost::weak_ptr<SphereButton> SphereButtonWeakPtr;
}

#endif
