// Copyright (C) Infiscape Corporation 2005-2007

#include <boost/bind.hpp>

#include <IOV/User.h>


namespace inf
{

User::User()
{
   /* Do nothing. */ ;
}

UserPtr User::create()
{
   return UserPtr(new User());
}

UserPtr User::init(ViewerPtr viewer)
{
   mViewPlatform.platformMoved().connect(boost::bind(&User::platformMoved,
                                                     this, viewer));

   mInterfaceTrader.init(viewer);

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
