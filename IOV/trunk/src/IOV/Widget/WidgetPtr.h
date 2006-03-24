// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_WIDGET_PTR_H_
#define _INF_WIDGET_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class Widget;
   typedef boost::shared_ptr<Widget> WidgetPtr;
   typedef boost::weak_ptr<Widget> WidgetWeakPtr;
}

#endif
