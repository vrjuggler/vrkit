#ifndef SCENE_PTR_H_
#define SCENE_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
namespace inf
{
   class Scene;
   typedef boost::shared_ptr<Scene> ScenePtr;
   typedef boost::weak_ptr<Scene> SceneWeakPtr;
}

#endif

