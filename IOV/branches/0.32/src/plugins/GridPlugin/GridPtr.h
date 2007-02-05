// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_GRID_PTR_H_
#define _INF_GRID_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class Grid;
   typedef boost::shared_ptr<Grid> GridPtr;
   typedef boost::weak_ptr<Grid> GridWeakPtr;   
}

#endif

