#ifndef SCENE_DATA_H_
#define SCENE_DATA_H_

#include <OpenSG/VRJ/Viewer/IOV/Config.h>

#include <boost/enable_shared_from_this.hpp>

#include <OpenSG/VRJ/Viewer/IOV/SceneDataPtr.h>


namespace inf
{

/** @interface */
class IOV_CLASS_API SceneData
   : public boost::enable_shared_from_this<SceneData>
{
public:
   virtual ~SceneData();

protected:
   SceneData();
};

}

#endif
