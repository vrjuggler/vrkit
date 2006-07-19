// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_PICK_PLUGIN_H_
#define _INF_PICK_PLUGIN_H_

#include <IOV/Plugin/PluginConfig.h>

#include <string>
#include <map>
#include <vector>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <gmtl/Matrix.h>

#include <snx/SoundHandle.h>

#include <IOV/EventDataPtr.h>
#include <IOV/Plugin.h>
#include <IOV/WandInterfacePtr.h>
#include <IOV/PluginFactory.h>
#include <IOV/Util/DigitalCommand.h>
#include <IOV/Grab/IntersectionStrategy.h>
#include <IOV/SceneObjectPtr.h>


namespace inf
{

class PickPlugin
   : public inf::Plugin
   , public boost::enable_shared_from_this<PickPlugin>
{
public:
   static inf::PluginPtr create()
   {
      return inf::PluginPtr(new PickPlugin());
   }

   virtual ~PickPlugin()
   {
      /* Do nothing. */ ;
   }

   virtual std::string getDescription()
   {
      return std::string("Picking");
   }

   virtual PluginPtr init(inf::ViewerPtr viewer);

   virtual void update(inf::ViewerPtr viewer);

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

   PickPlugin();

   void focusChanged(inf::ViewerPtr viewer);

   int defaultObjectMovedSlot(SceneObjectPtr obj, const gmtl::Matrix44f& newObjMat);

private:
   static std::string getElementType()
   {
      return std::string("iov_pick_plugin");
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

   WandInterfacePtr mWandInterface;

   /** Button for picking and releasing objects. */
   inf::DigitalCommand mPickBtn;
   std::string mPickText;

   bool mIntersecting;
   bool mPicking;
   SceneObjectPtr mIntersectedObj;
   SceneObjectPtr mPickedObj;

   snx::SoundHandle mIntersectSound;
   snx::SoundHandle mPickSound;

   std::vector<std::string> mStrategyPluginPath;

   inf::PluginFactoryPtr mPluginFactory;

   /** @name Intersection Strategy */
   //@{
   gmtl::Point3f mIntersectPoint;
   boost::signals::connection mIsectConnection;
   boost::signals::connection mDeIsectConnection;
   //@}

   EventDataPtr mEventData;
};

}


#endif
