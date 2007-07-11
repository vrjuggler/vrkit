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

void SignalRepository::addSignal(const std::string& id,
                                 inf::SignalContainerBasePtr container)
{
   if ( hasSignal(id) )
   {
      std::ostringstream msg_stream;
      msg_stream << "Signal already registered under '" << id << "'";
      throw inf::Exception(msg_stream.str(), IOV_LOCATION);
   }

   mSignals[id] = container;
}

void SignalRepository::removeSignal(const std::string& id)
{
   if ( hasSignal(id) )
   {
      mSignals.erase(id);
   }
}

SignalContainerBasePtr SignalRepository::getBaseSignal(const std::string& id)
{
   if ( ! hasSignal(id) )
   {
      std::ostringstream msg_stream;
      msg_stream << "Unknown signal identifier " << id;
      throw std::invalid_argument(msg_stream.str());
   }

   return mSignals[id];
}

}
