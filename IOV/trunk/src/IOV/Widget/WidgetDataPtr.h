// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_WIDGET_DATA_PTR_H_
#define _INF_WIDGET_DATA_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class WidgetData;
   typedef boost::shared_ptr<WidgetData> WidgetDataPtr;
   typedef boost::weak_ptr<WidgetData> WidgetDataWeakPtr;   
}

#endif
