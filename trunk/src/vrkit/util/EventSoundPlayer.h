// vrkit is (C) Copyright 2005-2008
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

#ifndef _VRKIT_UTIL_EVENT_SOUND_PLAYER_H_
#define _VRKIT_UTIL_EVENT_SOUND_PLAYER_H_

#include <vrkit/Config.h>

#include <string>
#include <boost/enable_shared_from_this.hpp>
#include <boost/signals/connection.hpp>

#include <snx/SoundHandle.h>

#include <jccl/Config/ConfigElementPtr.h>

#include <vrkit/SceneObjectPtr.h>
#include <vrkit/ViewerPtr.h>
#include <vrkit/scenedata/Event.h>
#include <vrkit/util/EventSoundPlayerPtr.h>


namespace vrkit
{

namespace util
{

/** \class EventSoundPlayer EventSoundPlayer.h vrkit/util/EventSoundPlayer.h
 *
 * Plays sounds in response to signals emitted by vrkit::EventData.
 *
 * @note This class was moved into the vrkit::util namespace in version 0.47.
 *
 * @since 0.26
 */
class VRKIT_CLASS_API EventSoundPlayer
   : public boost::enable_shared_from_this<EventSoundPlayer>
{
protected:
   EventSoundPlayer();

public:
   static EventSoundPlayerPtr create()
   {
      return EventSoundPlayerPtr(new EventSoundPlayer());
   }

   /**
    * Disconnects the slots of this object from their respective signal.
    *
    * @post The slots of this object are disconnected.
    */
   ~EventSoundPlayer();

   /**
    * Initializes and configures this sound player.
    *
    * @post The sound handles are initialized. The slots of this object are
    *       connected to the relevant signals in vrkit::EventData.
    *
    * @param viewer The VR Juggler application object within which this object
    *               will be used.
    */
   EventSoundPlayerPtr init(ViewerPtr viewer);

private:
   static std::string getElementType()
   {
      return std::string("event_sound_player");
   }

   /**
    *
    * @throw vrkit::Exception Thrown if configuration fails.
    */
   void configure(jccl::ConfigElementPtr cfgElt);

   /** @name Intersection */
   //@{
   /**
    * Object intersection slot; triggers \c mIntersectSound when invoked.
    *
    * @return vrkit::event::CONTINUE is returned.
    */
   event::ResultType objectIntersected();

   std::string                mIntersectSoundName;
   snx::SoundHandle           mIntersectSound;
   boost::signals::connection mIntersectSlotConnection;
   //@}

   /** @name Select */
   //@{
   /**
    * Object select slot; triggers \c mSelectSound when invoked.
    *
    * @return vrkit::event::CONTINUE is returned.
    */
   event::ResultType objectsSelected();

   std::string                mSelectSoundName;
   snx::SoundHandle           mSelectSound;
   boost::signals::connection mSelectSlotConnection;
   //@}
};

}

}


#endif /* _VRKIT_UTIL_EVENT_SOUND_PLAYER_H_ */
