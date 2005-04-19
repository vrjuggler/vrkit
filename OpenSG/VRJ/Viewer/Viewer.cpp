#include <OpenSG/VRJ/Viewer/Viewer.h>
#include <OpenSG/VRJ/Viewer/User.h>

namespace inf
{

void Viewer::init()
{
   vrj::OpenSGApp::init();

   // Create an initialize the user
   mUser = User::create();
   mUser->init();

   // Create and initialize the base scene object
   mScene = Scene::create();
   mScene->init();
}


}
