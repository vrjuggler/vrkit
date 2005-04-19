#ifndef USER_PTR_H_
#define USER_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
namespace inf
{
   class User;
   typedef boost::shared_ptr<User> UserPtr;
   typedef boost::weak_ptr<User> UserWeakPtr;   
}

#endif

