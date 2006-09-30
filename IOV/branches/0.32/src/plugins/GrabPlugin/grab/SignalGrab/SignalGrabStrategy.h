// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_SIGNAL_GRAB_STRATEGY_H_
#define _INF_SIGNAL_GRAB_STRATEGY_H_

#include <string>
#include <vector>
#include <boost/enable_shared_from_this.hpp>
#include <boost/signals/connection.hpp>

#include <jccl/Config/ConfigElementPtr.h>

#include <IOV/GrabSignalDataPtr.h>
#include <IOV/SceneObjectPtr.h>
#include <IOV/WandInterfacePtr.h>
#include <IOV/Grab/GrabStrategy.h>
#include <IOV/Util/DigitalCommand.h>


namespace inf
{

class SignalGrabStrategy
   : public inf::GrabStrategy
   , public boost::enable_shared_from_this<SignalGrabStrategy>
{
protected:
   SignalGrabStrategy();

public:
   static GrabStrategyPtr create()
   {
      return GrabStrategyPtr(new SignalGrabStrategy());
   }

   virtual ~SignalGrabStrategy();

   virtual GrabStrategyPtr init(ViewerPtr viewer, grab_callback_t grabCallback,
                                release_callback_t releaseCallback);

   virtual void setFocus(ViewerPtr viewer, const bool focused);

   virtual void update(ViewerPtr viweer);

   virtual std::vector<SceneObjectPtr> getGrabbedObjects();

   /**
    * Invokes the global scope delete operator.  This is required for proper
    * releasing of memory in DLLs on Windows.
    */
   void operator delete(void* p)
   {
      ::operator delete(p);
   }

protected:
   /**
    * Deletes this object.  This is an implementation of the pure virtual
    * inf::GrabStrategy::destroy() method.
    */
   virtual void destroy()
   {
      delete this;
   }

private:
   static std::string getElementType()
   {
      return std::string("signal_grab_strategy");
   }

   void configure(jccl::ConfigElementPtr elt);

   void grab();

   void release();

   /** @name inf::GrabSignalData Slots */
   //@{
   /**
    * Receives "asynchronous" object release signals in order to maintain
    * internal state correctly.
    *
    * @param objs The scene objects that were released asynchronously.
    *
    * @see inf::GrabSignalData::asyncRelease
    */
   void objectsReleased(const std::vector<SceneObjectPtr>& objs);
   //@}

   std::vector<int> transformButtonVec(const std::vector<int>& btns);

   GrabSignalDataPtr mGrabSignalData;

   WandInterfacePtr mWandInterface;

   /** @name Grab/Release Callbacks */
   //@{
   grab_callback_t    mGrabCallback;
   release_callback_t mReleaseCallback;
   //@}

   /** @name Button(s) for choosing objects to grab. */
   //@{
   inf::DigitalCommand mChooseBtn;
   std::string mChooseText;
   //@}

   /** @name Button(s) for grabbing objects. */
   //@{
   inf::DigitalCommand mGrabBtn;
   std::string mGrabText;
   //@}

   /** @name Button(s) for releasing objects. */
   //@{
   inf::DigitalCommand mReleaseBtn;
   std::string mReleaseText;
   //@}

   /**
    * Indicates whether the grab and release operation should behave as a
    * toggled state.
    */
   bool mGrabReleaseToggle;

   /** @name Grab State */
   //@{
   bool mGrabbing;
   std::vector<SceneObjectPtr> mGrabbedObjects;
   //@}

   std::vector<boost::signals::connection> mConnections;
};

}


#endif /* _INF_SIGNAL_GRAB_STRATEGY_H_ */
