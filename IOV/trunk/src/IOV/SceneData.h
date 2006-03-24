// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_SCENE_DATA_H_
#define _INF_SCENE_DATA_H_

#include <IOV/Config.h>

#include <boost/enable_shared_from_this.hpp>

#include <IOV/SceneDataPtr.h>


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
