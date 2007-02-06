// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_WIDGET_SCROLL_BAR_PTR_H_
#define _INF_WIDGET_SCROLL_BAR_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class ScrollBar;
   typedef boost::shared_ptr<ScrollBar> ScrollBarPtr;
   typedef boost::weak_ptr<ScrollBar> ScrollBarWeakPtr;
}

#endif
