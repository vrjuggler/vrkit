#ifndef SCENE_DATA_H_
#define SCENE_DATA_H_

#include <boost/enable_shared_from_this.hpp>

#include <OpenSG/VRJ/Viewer/SceneDataPtr.h>


namespace inf
{

/** @interface */
class SceneData : public boost::enable_shared_from_this<SceneData>
{
public:
   virtual ~SceneData();

protected:
   SceneData();
};

}

#endif
