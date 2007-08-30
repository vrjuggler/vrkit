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

#ifndef _VRKIT_STATUS_H_
#define _VRKIT_STATUS_H_

#include <vrkit/Config.h>

#include <string>
#include <iostream>
#include <sstream>
#include <boost/shared_ptr.hpp>
#include <boost/signal.hpp>
#include <boost/signals/connection.hpp>

#include <vpr/Util/Singleton.h>


namespace vrkit
{

/**
 * Status output class.
 */
class VRKIT_CLASS_API Status
{
vprSingletonHeader(Status);

   typedef boost::signal<void (const std::string&)> signal_type;

public:
   /** Typedef for status output slot type. */
   typedef signal_type::slot_type status_func_t;

protected:
   Status();

public:
   /** Write a status message.. */
   void writeStatusMsg(const std::string& msg);

   /** Registery a listener that will be called with status updates. */
   boost::signals::connection addOutputter(status_func_t newFunc);

public:
   /** Allows writing to status as a stream.
    * Uses a guard idiom for status output.
    */
   struct VRKIT_CLASS_API StatusStreamer
   {
      StatusStreamer(Status* status);

      StatusStreamer(const StatusStreamer& rhs)
         : mStream(rhs.mStream)
         , mStatus(rhs.mStatus)
      {
         /* Do nothing. */ ;
      }

      ~StatusStreamer();

      std::ostringstream* stream()
      {
         return mStream.get();
      }

    private:
       boost::shared_ptr<std::ostringstream> mStream; /**< Stream to use. */
       Status*             mStatus;    /**< Status we are to call. */
   };

   StatusStreamer getStreamer()
   {
      return StatusStreamer(this);
   }

private:
   /** Signal emitted when a statuc message is received. */
   signal_type mOutputSignal;
};

}  // namespace vrkit

#define VRKIT_STATUS (*(vrkit::Status::instance()->getStreamer().stream()))

#endif /* _VRKIT_STATUS_H_ */
