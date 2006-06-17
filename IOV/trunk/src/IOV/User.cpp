// Copyright (C) Infiscape Corporation 2005-2006

#include <boost/bind.hpp>

#include <IOV/User.h>


namespace inf
{

UserPtr User::create()
{
   UserPtr new_user(new User);
   return new_user;
}

UserPtr User::init(ViewerPtr viewer)
{
   mViewPlatform.platformMoved().connect(boost::bind(&User::platformMoved,
                                                     this, viewer));
   return shared_from_this();
}

void User::update(ViewerPtr)
{
   /* Do nothing. */ ;
}

void User::platformMoved(inf::ViewerPtr viewer)
{
   mViewPlatform.update(viewer);
}

}
