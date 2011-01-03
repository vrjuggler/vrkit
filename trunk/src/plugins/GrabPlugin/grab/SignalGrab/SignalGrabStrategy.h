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

#ifndef _VRKIT_SIGNAL_GRAB_STRATEGY_H_
#define _VRKIT_SIGNAL_GRAB_STRATEGY_H_

#include <string>
#include <vector>
#include <boost/enable_shared_from_this.hpp>
#include <boost/signals/connection.hpp>

#include <jccl/Config/ConfigElementPtr.h>

#include <vrkit/SceneObjectPtr.h>
#include <vrkit/WandInterfacePtr.h>
#include <vrkit/scenedata/GrabSignalDataPtr.h>
#include <vrkit/grab/Strategy.h>
#include <vrkit/util/DigitalCommand.h>


namespace vrkit
{

class SignalGrabStrategy
   : public grab::Strategy
   , public boost::enable_shared_from_this<SignalGrabStrategy>
{
protected:
   SignalGrabStrategy(const plugin::Info& info);

public:
   static grab::StrategyPtr create(const plugin::Info& info)
   {
      return grab::StrategyPtr(new SignalGrabStrategy(info));
   }

   virtual ~SignalGrabStrategy();

   virtual grab::StrategyPtr init(ViewerPtr viewer,
                                  grab_callback_t grabCallback,
                                  release_callback_t releaseCallback);

   virtual void setFocus(ViewerPtr viewer, const bool focused);

   virtual void update(ViewerPtr viweer);

   virtual std::vector<SceneObjectPtr> getGrabbedObjects();

private:
   static std::string getElementType()
   {
      return std::string("signal_grab_strategy");
   }

   void configure(jccl::ConfigElementPtr elt);

   void grab();

   void release();

   /** @name vrkit::GrabSignalData Slots */
   //@{
   /**
    * Receives "asynchronous" object release signals in order to maintain
    * internal state correctly.
    *
    * @param objs The scene objects that were released asynchronously.
    *
    * @see vrkit::GrabSignalData::asyncRelease
    */
   void objectsReleased(const std::vector<SceneObjectPtr>& objs);
   //@}

   GrabSignalDataPtr mGrabSignalData;

   WandInterfacePtr mWandInterface;

   /** @name Grab/Release Callbacks */
   //@{
   grab_callback_t    mGrabCallback;
   release_callback_t mReleaseCallback;
   //@}

   /** @name Button(s) for choosing objects to grab. */
   //@{
   util::DigitalCommand mChooseBtn;
   std::string mChooseText;
   //@}

   /** @name Button(s) for grabbing objects. */
   //@{
   util::DigitalCommand mGrabBtn;
   std::string mGrabText;
   //@}

   /** @name Button(s) for releasing objects. */
   //@{
   util::DigitalCommand mReleaseBtn;
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


#endif /* _VRKIT_SIGNAL_GRAB_STRATEGY_H_ */
