// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_EVENT_SOUND_PLAYER_PTR_H_
#define _INF_EVENT_SOUND_PLAYER_PTR_H_

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace inf
{
   class EventSoundPlayer;
   typedef boost::shared_ptr<EventSoundPlayer> EventSoundPlayerPtr;
   typedef boost::weak_ptr<EventSoundPlayer> EventSoundPlayerWeakPtr;   
}

#endif
