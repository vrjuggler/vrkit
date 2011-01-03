// vrkit is (C) Copyright 2005-2011
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

#ifndef _VRKIT_PICK_PLUGIN_H_
#define _VRKIT_PICK_PLUGIN_H_

#include <vrkit/plugin/Config.h>

#include <string>
#include <vector>
#include <boost/enable_shared_from_this.hpp>
#include <gmtl/Matrix.h>

#include <snx/SoundHandle.h>

#include <vrkit/SceneObjectPtr.h>
#include <vrkit/WandInterfacePtr.h>
#include <vrkit/scenedata/EventDataPtr.h>
#include <vrkit/util/DigitalCommand.h>
#include <vrkit/isect/Strategy.h>
#include <vrkit/viewer/Plugin.h>


namespace vrkit
{

class PickPlugin
   : public viewer::Plugin
   , public boost::enable_shared_from_this<PickPlugin>
{
protected:
   PickPlugin(const plugin::Info& info);

public:
   static viewer::PluginPtr create(const plugin::Info& info)
   {
      return viewer::PluginPtr(new PickPlugin(info));
   }

   virtual ~PickPlugin()
   {
      /* Do nothing. */ ;
   }

   virtual std::string getDescription()
   {
      return std::string("Picking");
   }

   virtual viewer::PluginPtr init(ViewerPtr viewer);

   virtual void update(ViewerPtr viewer);

   bool config(jccl::ConfigElementPtr elt);

protected:
   void focusChanged(ViewerPtr viewer);

   int defaultObjectMovedSlot(SceneObjectPtr obj,
                              const gmtl::Matrix44f& newObjMat);

private:
   static std::string getElementType()
   {
      return std::string("vrkit_pick_plugin");
   }

   /**
    * Responds to object intersection signals. If \p obj is newly intersected,
    * then our intersection highlight is applied to it.
    *
    * @post Our intersection highlight is applied to \p obj.
    *
    * @param obj         The object associated with the intersection signal.
    * @param pnt         The intersection point.
    */
   event::ResultType objectIntersected(SceneObjectPtr obj, gmtl::Point3f pnt);

   /**
    * Responds to object de-intersection signals. Our intersection highlight
    * is removed from \p obj.
    *
    * @post Our intersection highlight is removed from \p obj.
    *
    * @param obj         The object associated with the intersection signal.
    */
   event::ResultType objectDeintersected(SceneObjectPtr obj);

   WandInterfacePtr mWandInterface;

   /** Button for picking and releasing objects. */
   util::DigitalCommand mPickBtn;
   std::string mPickText;

   bool mIntersecting;
   bool mPicking;
   SceneObjectPtr mIntersectedObj;
   SceneObjectPtr mPickedObj;

   snx::SoundHandle mIntersectSound;
   snx::SoundHandle mPickSound;

   std::vector<std::string> mStrategyPluginPath;

   /** @name Intersection Strategy */
   //@{
   gmtl::Point3f mIntersectPoint;
   boost::signals::connection mIsectConnection;
   boost::signals::connection mDeIsectConnection;
   //@}

   EventDataPtr mEventData;
};

}


#endif /* _VRKIT_PICK_PLUGIN_H_ */
