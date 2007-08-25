// vrkit is (C) Copyright 2005-2007
//    by Allen Bierbaum, Aron Bierbuam, Patrick Hartling, and Daniel Shipton
//
// This file is part of vrkit.
//
// vrkit is free software; you can redistribute it and/or modify it under the
// terms of the GNU Lesser General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// vrkit is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
// more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

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

float User::getInterocularDistance(const float scale) const
{
   return mVrjUser->getInterocularDistance() * scale;
}

void User::platformMoved(inf::ViewerPtr viewer)
{
   mViewPlatform.update(viewer);
}

}
