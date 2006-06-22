// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_EVENT_SOUND_PLAYER_H_
#define _INF_EVENT_SOUND_PLAYER_H_

#include <IOV/Config.h>

#include <string>
#include <boost/enable_shared_from_this.hpp>
#include <boost/signals/connection.hpp>

#include <snx/SoundHandle.h>

#include <jccl/Config/ConfigElementPtr.h>

#include <IOV/Event.h>
#include <IOV/SceneObjectPtr.h>
#include <IOV/ViewerPtr.h>
#include <IOV/Util/EventSoundPlayerPtr.h>


namespace inf
{

/**
 * Plays sounds in response to signals emitted by inf::EventData.
 *
 * @since 0.26
 */
class IOV_CLASS_API EventSoundPlayer
   : public boost::enable_shared_from_this<EventSoundPlayer>
{
public:
   static inf::EventSoundPlayerPtr create()
   {
      return inf::EventSoundPlayerPtr(new EventSoundPlayer());
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
    *       connected to the relevant signals in inf::EventData.
    *
    * @param viewer The VR Juggler application object within which this object
    *               will be used.
    */
   inf::EventSoundPlayerPtr init(inf::ViewerPtr viewer);

protected:
   EventSoundPlayer();

private:
   static std::string getElementType()
   {
      return std::string("event_sound_player");
   }

   /**
    *
    * @throw inf::Exception is thrown if configuration fails.
    */
   void configure(jccl::ConfigElementPtr cfgElt);

   /** @name Intersection */
   //@{
   /**
    * Object intersection slot; triggers \c mIntersectSound when invoked.
    *
    * @param sceneObj The intersected scene object. This parameter is ignored.
    *
    * @return inf::Event::CONTINUE is returned.
    */
   inf::Event::ResultType objectIntersected(inf::SceneObjectPtr sceneObj);

   std::string                mIntersectSoundName;
   snx::SoundHandle           mIntersectSound;
   boost::signals::connection mIntersectSlotConnection;
   //@}

   /** @name Select */
   //@{
   /**
    * Object select slot; triggers \c mSelectSound when invoked.
    *
    * @param sceneObjs The selected scene objects. This parameter is ignored.
    *
    * @return inf::Event::CONTINUE is returned.
    */
   inf::Event::ResultType
      objectsSelected(const std::vector<inf::SceneObjectPtr>& sceneObjs);

   std::string                mSelectSoundName;
   snx::SoundHandle           mSelectSound;
   boost::signals::connection mSelectSlotConnection;
   //@}
};

}


#endif /* _INF_EVENT_SOUND_PLAYER_H_ */
