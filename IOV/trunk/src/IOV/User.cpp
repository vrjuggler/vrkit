// Copyright (C) Infiscape Corporation 2005-2007

#include <boost/bind.hpp>

#include <vrj/Kernel/User.h>

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

UserPtr User::init(ViewerPtr viewer, vrj::UserPtr user)
{
   mViewPlatform.platformMoved().connect(boost::bind(&User::platformMoved,
                                                     this, viewer));

   mVrjUser = user;
   mInterfaceTrader.init(viewer);

   return shared_from_this();
}

void User::update(ViewerPtr)
{
   /* Do nothing. */ ;
}

gadget::PositionProxyPtr User::getHeadProxy() const
{
   return mVrjUser->getHeadPosProxy();
}

float User::getInterocularDistance(float scale) const
{
   return mVrjUser->getInterocularDistance() * scale;
}

void User::platformMoved(inf::ViewerPtr viewer)
{
   mViewPlatform.update(viewer);
}

}
