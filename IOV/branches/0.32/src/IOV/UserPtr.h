// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_USER_PTR_H_
#define _INF_USER_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
namespace inf
{
   class User;
   typedef boost::shared_ptr<User> UserPtr;
   typedef boost::weak_ptr<User> UserWeakPtr;   
}

#endif

