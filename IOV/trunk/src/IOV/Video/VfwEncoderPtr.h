// Copyright (C) Infiscape Corporation 2005-2007
#ifdef IOV_WITH_VFW

#ifndef _INF_VFW_ENCODER_PTR_H_
#define _INF_VFW_ENCODER_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class VfwEncoder;
   typedef boost::shared_ptr<VfwEncoder> VfwEncoderPtr;
   typedef boost::weak_ptr<VfwEncoder> VfwEncoderWeakPtr;
}

#endif /*_INF_VFW_ENCODER_PTR_H_*/

#endif /*IOV_WITH_VFW*/
