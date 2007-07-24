// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_STATUS_H_
#define _INF_STATUS_H_

#include <IOV/Config.h>

#include <string>
#include <iostream>
#include <sstream>
#include <boost/shared_ptr.hpp>
#include <boost/signal.hpp>
#include <boost/signals/connection.hpp>

#include <vpr/Util/Singleton.h>


namespace inf
{

/**
 * Status output class.
 */
class IOV_CLASS_API Status
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
   struct IOV_CLASS_API StatusStreamer
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

}  // namespace inf

#define IOV_STATUS (*(inf::Status::instance()->getStreamer().stream()))

#endif
