// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_SCENE_DATA_PTR_H_
#define _INF_SCENE_DATA_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
namespace inf
{
   class SceneData;
   typedef boost::shared_ptr<SceneData> SceneDataPtr;
   typedef boost::weak_ptr<SceneData> SceneDataWeakPtr;
}

#endif

