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

#include <vrkit/Config.h>

#include <boost/bind.hpp>

#include <OpenSG/OSGConfig.h>

#include <jccl/Config/ConfigElement.h>

#include <vrkit/Scene.h>
#include <vrkit/Viewer.h>
#include <vrkit/Exception.h>
#include <vrkit/scenedata/EventData.h>

#include <vrkit/util/EventSoundPlayer.h>


namespace vrkit
{

namespace util
{

EventSoundPlayer::EventSoundPlayer()
   : mIntersectSoundName("intersect")
   , mSelectSoundName("select")
{
   /* Do nothing. */ ;
}

EventSoundPlayer::~EventSoundPlayer()
{
   mIntersectSlotConnection.disconnect();
   mSelectSlotConnection.disconnect();
}

EventSoundPlayerPtr EventSoundPlayer::init(ViewerPtr viewer)
{

   jccl::ConfigElementPtr cfg_elt =
      viewer->getConfiguration().getConfigElement(getElementType());

   if ( cfg_elt )
   {
      try
      {
         // Configure ourselves.
         configure(cfg_elt);
      }
      catch (Exception& ex)
      {
         std::cerr << "Configuration of EventSoundPlayer failed:\n"
                   << ex.what() << std::endl;
      }
   }

   mIntersectSound.init(mIntersectSoundName);
   mSelectSound.init(mSelectSoundName);

   EventDataPtr event_data = viewer->getSceneObj()->getSceneData<EventData>();

   mIntersectSlotConnection =
      event_data->objectIntersected.connect(
         boost::bind(&EventSoundPlayer::objectIntersected, this)
      );
   mSelectSlotConnection =
      event_data->objectsSelected.connect(
         boost::bind(&EventSoundPlayer::objectsSelected, this)
      );

   return shared_from_this();
}

void EventSoundPlayer::configure(jccl::ConfigElementPtr cfgElt)
{
   vprASSERT(cfgElt->getID() == getElementType());

   const unsigned int req_cfg_version(1);

   // Check for correct version of plugin configuration.
   if ( cfgElt->getVersion() < req_cfg_version )
   {
      std::stringstream msg;
      msg << "Configuration of EventSoundPlayer failed.  Required config "
          << "element version is " << req_cfg_version << ", but element '"
          << cfgElt->getName() << "' is version " << cfgElt->getVersion();
      throw Exception(msg.str(), VRKIT_LOCATION);
   }

   const std::string intersect_name_prop("intersect_sound_name");
   const std::string select_name_prop("select_sound_name");

   mIntersectSoundName = cfgElt->getProperty<std::string>(intersect_name_prop);
   mSelectSoundName    = cfgElt->getProperty<std::string>(select_name_prop);
}

event::ResultType EventSoundPlayer::objectIntersected()
{
   mIntersectSound.trigger();
   return event::CONTINUE;
}

event::ResultType EventSoundPlayer::objectsSelected()
{
   mSelectSound.trigger();
   return event::CONTINUE;
}

}

}
