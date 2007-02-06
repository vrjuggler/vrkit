// Copyright (C) Infiscape Corporation 2005-2007

#include <IOV/Config.h>

#include <boost/bind.hpp>

#include <OpenSG/OSGConfig.h>

#include <jccl/Config/ConfigElement.h>

#include <IOV/EventData.h>
#include <IOV/Viewer.h>
#include <IOV/Util/Exceptions.h>

#include <IOV/Util/EventSoundPlayer.h>


namespace inf
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

inf::EventSoundPlayerPtr EventSoundPlayer::init(inf::ViewerPtr viewer)
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
      catch (inf::Exception& ex)
      {
         std::cerr << "Configuration of EventSoundPlayer failed:\n"
                   << ex.what() << std::endl;
      }
   }

   mIntersectSound.init(mIntersectSoundName);
   mSelectSound.init(mSelectSoundName);

   inf::EventDataPtr event_data =
      viewer->getSceneObj()->getSceneData<inf::EventData>();

   mIntersectSlotConnection =
      event_data->mObjectIntersectedSignal.connect(
         boost::bind(&EventSoundPlayer::objectIntersected, this)
      );
   mSelectSlotConnection =
      event_data->mObjectsSelectedSignal.connect(
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
      throw Exception(msg.str(), IOV_LOCATION);
   }

   const std::string intersect_name_prop("intersect_sound_name");
   const std::string select_name_prop("select_sound_name");

   mIntersectSoundName = cfgElt->getProperty<std::string>(intersect_name_prop);
   mSelectSoundName    = cfgElt->getProperty<std::string>(select_name_prop);
}

inf::Event::ResultType EventSoundPlayer::objectIntersected()
{
   mIntersectSound.trigger();
   return inf::Event::CONTINUE;
}

inf::Event::ResultType EventSoundPlayer::objectsSelected()
{
   mSelectSound.trigger();
   return inf::Event::CONTINUE;
}

}
