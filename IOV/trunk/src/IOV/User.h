// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_USER_H_
#define _INF_USER_H_

#include <IOV/Config.h>

#include <boost/enable_shared_from_this.hpp>

#include <IOV/UserPtr.h>
#include <IOV/InterfaceTrader.h>
#include <IOV/ViewPlatform.h>
#include <IOV/ViewerPtr.h>

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


namespace inf
{

class Configuration;

/**
 * This class holds all information related to a user of the application.
 * This includes devices they are using and their position in the virtual
 * world.
 *
 * See inf::Scene for definition of the coordinate frames used.
 *
 * @see inf::Scene
 */
class IOV_CLASS_API User : public boost::enable_shared_from_this<User>
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


#endif
