// Copyright (C) Infiscape Corporation 2005-2007

#include <sstream>

#include <IOV/Util/Exceptions.h>
#include <IOV/SignalRepository.h>

namespace inf
{

const vpr::GUID SignalRepository::type_guid(
   "1912103d-f3c8-4129-8555-ea3d58f69c1f"
);

SignalRepository::SignalRepository()
{
   /* Do nothing. */ ;
}

SignalRepository::~SignalRepository()
{
   /* Do nothing. */ ;
}

void SignalRepository::addSignal(const std::string& id)
{
   if ( hasSignal(id) )
   {
      std::ostringstream msg_stream;
      msg_stream << "Signal already registered under '" << id << "'";
      throw inf::Exception(msg_stream.str(), IOV_LOCATION);
   }

   mSignals[id] = boost::shared_ptr<signal_t>(new signal_t());
}

void SignalRepository::removeSignal(const std::string& id)
{
   if ( hasSignal(id) )
   {
      mSignals.erase(id);
   }
}

bool SignalRepository::hasSignal(const std::string& id) const
{
   return mSignals.count(id) > 0;
}

boost::signals::connection SignalRepository::
connect(const std::string& id, SignalRepository::signal_t::slot_type slot)
{
   if ( ! hasSignal(id) )
   {
      addSignal(id);
   }

   return mSignals[id]->connect(slot);
}

void SignalRepository::emit(const std::string& id)
{
   if ( ! hasSignal(id) )
   {
      std::ostringstream msg_stream;
      msg_stream << "No such signal '" << id << "'";
      throw inf::Exception(msg_stream.str(), IOV_LOCATION);
   }

   (*mSignals[id])(id);
}

}
