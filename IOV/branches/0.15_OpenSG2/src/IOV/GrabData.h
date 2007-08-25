// vrkit is (C) Copyright 2005-2007
//    by Allen Bierbaum, Aron Bierbuam, Patrick Hartling, and Daniel Shipton
//
// This file is part of vrkit.
//
// vrkit is free software; you can redistribute it and/or modify it under the
// terms of the GNU Lesser General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// vrkit is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
// more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
