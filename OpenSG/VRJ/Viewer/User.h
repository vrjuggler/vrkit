#ifndef USER_H
#define USER_H

#include <OpenSG/VRJ/Viewer/UserPtr.h>
#include <OpenSG/VRJ/Viewer/DevicesPtr.h>
#include <OpenSG/VRJ/Viewer/ViewPlatform.h>

#include <boost/enable_shared_from_this.hpp>

namespace inf
{

/**
 * This class holds all information related to a user of the application.
 * This includes devices they are using and their position in the virtual world.
 *
 * @see Scene for definition of the coordinate frames used.
 */
class User : public boost::enable_shared_from_this<User>
{
public:
   static UserPtr create();

   void init()
   {;
   }

protected:
   User()
   {;}

private:
   /** Devices abstraction for the user.
   * @link association */
   /*# Devices lnkDevices; */
   DevicesPtr     mDevices;

   ViewPlatform   mViewPlatform;    /** The user's view platform. */
};

}
#endif
