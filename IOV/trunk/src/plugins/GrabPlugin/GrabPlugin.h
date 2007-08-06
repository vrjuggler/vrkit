// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_GRAB_PLUGIN_H_
#define _INF_GRAB_PLUGIN_H_

#include <IOV/Plugin/PluginConfig.h>

#include <string>
#include <map>
#include <vector>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>

#include <gmtl/Matrix.h>
#include <gmtl/Point.h>

#include <IOV/EventData.h>
#include <IOV/Plugin.h>
#include <IOV/AbstractPluginPtr.h>
#include <IOV/WandInterfacePtr.h>
#include <IOV/Grab/GrabStrategyPtr.h>
#include <IOV/Grab/MoveStrategyPtr.h>
#include <IOV/SceneObjectPtr.h>


namespace inf
{

class GrabPlugin
   : public inf::Plugin
   , public boost::enable_shared_from_this<GrabPlugin>
{
protected:
   GrabPlugin(const inf::plugin::Info& info);

public:
   static inf::PluginPtr create(const inf::plugin::Info& info)
   {
      return inf::PluginPtr(new GrabPlugin(info));
   }

   virtual ~GrabPlugin();

   virtual std::string getDescription()
   {
      return std::string("Grabbing");
   }

   virtual PluginPtr init(inf::ViewerPtr viewer);

   virtual void update(inf::ViewerPtr viewer);

protected:
   void focusChanged(inf::ViewerPtr viewer);

   inf::Event::ResultType
      defaultObjectsMovedSlot(const EventData::moved_obj_list_t& objs);

private:
   static std::string getElementType()
   {
      return std::string("iov_grab_plugin");
   }

   bool config(jccl::ConfigElementPtr elt, inf::ViewerPtr viewer);

   std::vector<std::string> makeSearchPath(jccl::ConfigElementPtr elt,
                                           const std::string& prop,
                                           const std::string& subdir);

   void pluginInstantiated(inf::AbstractPluginPtr plugin,
                           inf::ViewerPtr viewer);

   void objectsGrabbed(inf::ViewerPtr viewer,
                       const std::vector<SceneObjectPtr>& objs,
                       const gmtl::Point3f& isectPoint);

   void objectsReleased(inf::ViewerPtr viewer,
                        const std::vector<SceneObjectPtr>& objs);

   /**
    * Responds to the signal emitted when the grabbable state of a scene
    * object changes. If \p obj is currently grabbed, then it is released.
    *
    * @param obj    The scene object that was removed from inf::GrabData.
    * @param viewer The VR Juggler application object within which this
    *               plug-in is active.
    *
    * @see objectsReleased()
    */
   void grabbableObjStateChanged(inf::SceneObjectPtr obj,
                                 inf::ViewerPtr viewer);

   WandInterfacePtr mWandInterface;

   /** @name Grab Strategy */
   //@{
   std::string     mGrabStrategyName;
   GrabStrategyPtr mGrabStrategy;
   //@}

   /** @name Move Strategies */
   //@{
   std::map<SceneObjectPtr, gmtl::Matrix44f> mGrabbed_pobj_M_obj_map;
   std::vector<MoveStrategyPtr> mMoveStrategies;
   std::vector<std::string> mMoveStrategyNames;
   //@}

   EventDataPtr mEventData;
};

}


#endif
