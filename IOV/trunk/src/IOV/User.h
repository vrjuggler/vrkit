// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_USER_H_
#define _INF_USER_H_

#include <IOV/UserPtr.h>
#include <IOV/InterfaceTrader.h>
#include <IOV/ViewPlatform.h>
#include <IOV/ViewerPtr.h>

#include <boost/enable_shared_from_this.hpp>


namespace inf
{

/**
 * This class holds all information related to a user of the application.
 * This includes devices they are using and their position in the virtual
 * world.
 *
 * See inf::Scene for definition of the coordinate frames used.
 *
 * @see inf::Scene
 */
class User : public boost::enable_shared_from_this<User>
{
public:
   static UserPtr create();

   /**
    * Initializes this user object.
    *
    * @return This object is returned as a shared pointer.
    */
   UserPtr init();

   /**
    * Updates the user and user-associated information.
    *
    * @post User and view platform are updated.
    */
   void update(ViewerPtr viewer);

   InterfaceTrader& getInterfaceTrader()
   {
      return mInterfaceTrader;
   }

   /** Returns the view platform that we are using. */
   ViewPlatform& getViewPlatform()
   {
      return mViewPlatform;
   }

protected:
   User()
   {;}

private:
   /** Devices abstraction for the user. */
   InterfaceTrader    mInterfaceTrader;

   ViewPlatform   mViewPlatform;    /**< The user's view platform. */
};

}


#endif
