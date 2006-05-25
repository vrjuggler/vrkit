// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_GRAB_PLUGIN_H_
#define _INF_GRAB_PLUGIN_H_

#include <IOV/Plugin/PluginConfig.h>

#include <string>
#include <map>
#include <vector>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <gmtl/Matrix.h>

#include <snx/SoundHandle.h>

#include <IOV/EventDataPtr.h>
#include <IOV/GrabDataPtr.h>
#include <IOV/Plugin.h>
#include <IOV/WandInterfacePtr.h>
#include <IOV/PluginFactory.h>
#include <IOV/Util/DigitalCommand.h>
#include <IOV/Grab/IntersectionStrategy.h>
#include <IOV/Grab/MoveStrategy.h>
#include <IOV/SceneObjectPtr.h>


namespace inf
{

class GrabPlugin
   : public inf::Plugin
   , public boost::enable_shared_from_this<GrabPlugin>
{
public:
   static inf::PluginPtr create()
   {
      return inf::PluginPtr(new GrabPlugin());
   }

   virtual ~GrabPlugin()
   {
      mIsectConnection.disconnect();
      mDeIsectConnection.disconnect();
      mGrabDataConnection.disconnect();
   }

   virtual std::string getDescription()
   {
      return std::string("Grabbing");
   }

   virtual PluginPtr init(inf::ViewerPtr viewer);

   virtual void updateState(inf::ViewerPtr viewer);

   virtual void run(inf::ViewerPtr viewer);

   bool config(jccl::ConfigElementPtr elt);

   /**
    * Invokes the global scope delete operator.  This is required for proper
    * releasing of memory in DLLs on Win32.
    */
   void operator delete(void* p)
   {
      ::operator delete(p);
   }

protected:
   /**
    * Deletes this object.  This is an implementation of the pure virtual
    * inf::Plugin::destroy() method.
    */
   virtual void destroy()
   {
      delete this;
   }

   GrabPlugin();

   void focusChanged(inf::ViewerPtr viewer);

   inf::Event::ResultType
      defaultObjectMovedSlot(SceneObjectPtr obj,
                             const gmtl::Matrix44f& newObjMat);

private:
   static std::string getElementType()
   {
      return std::string("iov_grab_plugin");
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
   inf::Event::ResultType objectIntersected(inf::SceneObjectPtr obj,
                                            inf::SceneObjectPtr parentObj,
                                            gmtl::Point3f pnt);

   /**
    * Responds to object de-intersection signals. Our intersection highlight
    * is removed from \p obj.
    *
    * @post Our intersection highlight is removed from \p obj.
    *
    * @param obj         The object associated with the intersection signal.
    */
   inf::Event::ResultType objectDeintersected(inf::SceneObjectPtr obj);

   /**
    * @post \c mGrabbing is false, and \c mGrabbedObj holds a NULL pointer.
    *
    * @param viewer The VR Juggler application object within which this
    *               plug-in is active.
    */
   void releaseGrabbedObject(inf::ViewerPtr viewer);

   /**
    * Responds to the signal emitted when an object is removed from
    * inf::GrabData. If \p obj is the object that is currently grabbed, then
    * the grabbed object is released.
    *
    * @param obj    The scene object that was removed from inf::GrabData.
    * @param viewer The VR Juggler application object within which this
    *               plug-in is active.
    *
    * @see releaseGrabbedObject()
    */
   void grabbableObjectRemoved(inf::SceneObjectPtr obj, inf::ViewerPtr viewer);

   WandInterfacePtr mWandInterface;

   /** Button for grabbing and releasing objects. */
   inf::DigitalCommand mGrabBtn;
   std::string mGrabText;

   /** @name Intersection Strategy */
   //@{
   bool mIntersecting;
   SceneObjectPtr mGrabbedObj;
   SceneObjectPtr mIntersectedObj;
   gmtl::Point3f mIntersectPoint;
   snx::SoundHandle mIntersectSound;
   boost::signals::connection mIsectConnection;
   boost::signals::connection mDeIsectConnection;
   //@}

   /** @name Grabbing data */
   //@{
   bool mGrabbing;
   snx::SoundHandle mGrabSound;
   GrabDataPtr  mGrabData;
   boost::signals::connection mGrabDataConnection;
   //@}

   std::vector<std::string> mStrategyPluginPath;

   inf::PluginFactoryPtr mPluginFactory;

   /** @name Move Strategy */
   //@{
   gmtl::Matrix44f mGrabbed_pobj_M_obj;
   std::map< std::string, boost::function<MoveStrategyPtr ()> > mMoveStrategyMap;
   std::vector<MoveStrategyPtr> mMoveStrategies;
   std::vector<std::string> mMoveStrategyNames;
   //@}

   EventDataPtr mEventData;
};

}


#endif
