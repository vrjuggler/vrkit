// Copyright (C) Infiscape Corporation 2005-2006

#include <IOV/User.h>

namespace inf
{

UserPtr User::create()
{
   UserPtr new_user(new User);
   return new_user;
}

UserPtr User::init()
{
   return shared_from_this();
}

void User::update(ViewerPtr viewer)
{
   mViewPlatform.update(viewer);
}

}

