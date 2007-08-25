// vrkit is (C) Copyright 2005-2007
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
