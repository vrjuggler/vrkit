// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_STATUS_PANEL_DATA_PTR_H_
#define _INF_STATUS_PANEL_DATA_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class StatusPanelData;
   typedef boost::shared_ptr<StatusPanelData> StatusPanelDataPtr;
   typedef boost::weak_ptr<StatusPanelData> StatusPanelDataWeakPtr;   
}

#endif /*_INF_STATUS_PANEL_DATA_PTR_H_*/
