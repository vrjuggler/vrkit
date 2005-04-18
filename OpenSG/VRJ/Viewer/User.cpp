#include <OpenSG/VRJ/Viewer/User.h>

namespace inf
{

UserPtr User::create()
{
   UserPtr new_user(new User);
   return new_user;
}

}

