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

#include <IOV/Status.h>


namespace
{

void write_to_cout(const std::string& msg)
{
   std::cout << msg << std::flush;
}

}  // namespace

namespace inf
{

vprSingletonImp(Status);


Status::Status()
{
   addOutputter(&write_to_cout);
}

void Status::writeStatusMsg(const std::string& msg)
{
   mOutputSignal(msg);
}

boost::signals::connection Status::addOutputter(status_func_t newFunc)
{
   return mOutputSignal.connect(newFunc);
}

Status::StatusStreamer::StatusStreamer(Status* status)
   : mStream(new std::ostringstream)
   , mStatus(status)
{
   /* Do nothing. */ ;
}

Status::StatusStreamer::~StatusStreamer()
{
   if ( mStream )
   {
      mStatus->writeStatusMsg(mStream->str());
   }

   mStatus = NULL;
}

} // namespace inf
