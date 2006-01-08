// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_GRAB_DATA_H_
#define _INF_GRAB_DATA_H_

#include <IOV/Config.h>

#include <OpenSG/OSGCoredNodePtr.h>
#include <OpenSG/OSGNode.h>
#include <OpenSG/OSGTransform.h>

#include <vpr/vpr.h>
#include <vpr/Util/GUID.h>

#include <IOV/SceneData.h>
#include <IOV/GrabDataPtr.h>


namespace inf
{

typedef OSG::TransformNodePtr CoredTransformPtr;

class IOV_CLASS_API GrabData : public inf::SceneData
{
public:
   typedef std::vector<OSG::TransformNodePtr> object_list_t;

   /**
    * The unique type identifier for scene data used by plug-ins that manage
    * grabbing of objects in the scene.
    */
   static const vpr::GUID type_guid;

   static GrabDataPtr create()
   {
      return GrabDataPtr(new GrabData);
   }

   virtual ~GrabData();

   void addObject(OSG::TransformNodePtr obj)
   {
      mObjects.push_back(obj);
   }

   const object_list_t& getObjects() const
   {
      return mObjects;
   }

protected:
   GrabData();

   object_list_t mObjects;
};

}


#endif
