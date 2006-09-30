// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_WIDGET_FRAME_PTR_H_
#define _INF_WIDGET_FRAME_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class Frame;
   typedef boost::shared_ptr<Frame> FramePtr;
   typedef boost::weak_ptr<Frame> FrameWeakPtr;
}

#endif
