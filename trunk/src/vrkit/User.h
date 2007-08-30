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

#ifndef _VRKIT_USER_H_
#define _VRKIT_USER_H_

#include <vrkit/Config.h>

#include <boost/enable_shared_from_this.hpp>

#include <vrkit/UserPtr.h>
#include <vrkit/InterfaceTrader.h>
#include <vrkit/ViewPlatform.h>
#include <vrkit/ViewerPtr.h>

#include <vrj/vrjParam.h>

#if __VJ_version >= 2003005
#  include <gadget/Type/PositionProxyPtr.h>
#  include <vrj/Kernel/UserPtr.h>
#else
namespace gadget
{

typedef class PositionProxy* PositionProxyPtr;

}

namespace vrj
{

typedef class User* UserPtr;
}
#endif


namespace vrkit
{

class Configuration;

/**
 * This class holds all information related to a user of the application.
 * This includes devices they are using and their position in the virtual
 * world.
 *
 * See vrkit::Scene for definition of the coordinate frames used.
 *
 * @see vrkit::Scene
 */
class VRKIT_CLASS_API User : public boost::enable_shared_from_this<User>
{
protected:
   User();

public:
   static UserPtr create();

   /**
    * Initializes this user object.
    *
    * @return This object is returned as a shared pointer.
    */
   UserPtr init(ViewerPtr viewer, vrj::UserPtr user);

   /**
    * Updates the user and user-associated information.
    */
   void update(ViewerPtr viewer);

   /**
    * Retrieves the position proxy for this user's head.
    *
    * @since 0.40.1
    */
   gadget::PositionProxyPtr getHeadProxy() const;

   /**
    * Retrieves the interocular distance for this
    * users eyes
    *
    * @since 0.40.2
    */
   float getInterocularDistance(const float scale = 1.0f) const;

   InterfaceTrader& getInterfaceTrader()
   {
      return mInterfaceTrader;
   }

   /** Returns the view platform that we are using. */
   ViewPlatform& getViewPlatform()
   {
      return mViewPlatform;
   }

private:
   void platformMoved(ViewerPtr viewer);

   vrj::UserPtr mVrjUser;

   /** Devices abstraction for the user. */
   InterfaceTrader    mInterfaceTrader;

   ViewPlatform   mViewPlatform;    /**< The user's view platform. */
};

}


#endif /* _VRKIT_USER_H_ */
