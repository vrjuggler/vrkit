#include <OpenSG/VRJ/Viewer/Scene.h>

namespace inf
{

ScenePtr Scene::create()
{
   ScenePtr new_scene(new Scene);
   return new_scene;
}

}

