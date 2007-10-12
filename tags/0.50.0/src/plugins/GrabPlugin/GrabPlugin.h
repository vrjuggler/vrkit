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

#ifndef _VRKIT_GRAB_PLUGIN_H_
#define _VRKIT_GRAB_PLUGIN_H_

#include <vrkit/plugin/Config.h>

#include <string>
#include <map>
#include <vector>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>

#include <gmtl/Matrix.h>
#include <gmtl/Point.h>

#include <vrkit/AbstractPluginPtr.h>
#include <vrkit/WandInterfacePtr.h>
#include <vrkit/SceneObjectPtr.h>
#include <vrkit/scenedata/EventData.h>
#include <vrkit/grab/StrategyPtr.h>
#include <vrkit/move/StrategyPtr.h>
#include <vrkit/viewer/Plugin.h>


namespace vrkit
{

class GrabPlugin
   : public viewer::Plugin
   , public boost::enable_shared_from_this<GrabPlugin>
{
protected:
   GrabPlugin(const plugin::Info& info);

public:
   static viewer::PluginPtr create(const plugin::Info& info)
   {
      return viewer::PluginPtr(new GrabPlugin(info));
   }

   virtual ~GrabPlugin();

   virtual std::string getDescription()
   {
      return std::string("Grabbing");
   }

   virtual viewer::PluginPtr init(ViewerPtr viewer);

   virtual void update(ViewerPtr viewer);

protected:
   void focusChanged(ViewerPtr viewer);

   event::ResultType
      defaultObjectsMovedSlot(const EventData::moved_obj_list_t& objs);

private:
   static std::string getElementType()
   {
      return std::string("vrkit_grab_plugin");
   }

   bool config(jccl::ConfigElementPtr elt, ViewerPtr viewer);

   std::vector<std::string> makeSearchPath(jccl::ConfigElementPtr elt,
                                           const std::string& prop,
                                           const std::string& subdir);

   void pluginInstantiated(AbstractPluginPtr plugin, ViewerPtr viewer);

   void objectsGrabbed(ViewerPtr viewer,
                       const std::vector<SceneObjectPtr>& objs,
                       const gmtl::Point3f& isectPoint);

   void objectsReleased(ViewerPtr viewer,
                        const std::vector<SceneObjectPtr>& objs);

   /**
    * Responds to the signal emitted when the grabbable state of a scene
    * object changes. If \p obj is currently grabbed, then it is released.
    *
    * @param obj    The scene object that was removed from vrkit::GrabData.
    * @param viewer The VR Juggler application object within which this
    *               plug-in is active.
    *
    * @see objectsReleased()
    */
   void grabbableObjStateChanged(SceneObjectPtr obj, ViewerPtr viewer);

   WandInterfacePtr mWandInterface;

   /** @name Grab Strategy */
   //@{
   std::string       mGrabStrategyName;
   grab::StrategyPtr mGrabStrategy;
   //@}

   /** @name Move Strategies */
   //@{
   std::map<SceneObjectPtr, gmtl::Matrix44f> mGrabbed_pobj_M_obj_map;
   std::vector<move::StrategyPtr> mMoveStrategies;
   std::vector<std::string> mMoveStrategyNames;
   //@}

   EventDataPtr mEventData;
};

}


#endif /* _VRKIT_GRAB_PLUGIN_H_ */
