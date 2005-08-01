// Copyright (C) Infiscape Corporation 2005

#ifndef _IOV_STATUS_H_
#define _IOV_STATUS_H_

#include <IOV/Config.h>

#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <vpr/Util/Singleton.h>


namespace inf
{

/** Status output class.
 */
class IOV_CLASS_API Status
{
vprSingletonHeader(Status);

public:
   /**< Typedef for status output functor type. */
   typedef boost::function<void (const std::string&)> status_func_t;

protected:
   Status();

public:
   /** Write a status message.. */
   void writeStatusMsg(const std::string& msg);

   /** Registery a listener that will be called with status updates. */
   void addOutputter(status_func_t newFunc);

public:
   /** Allows writing to status as a stream.
    * Uses a guard idiom for status output.
    */
   struct StatusStreamer
   {
      StatusStreamer(Status* status)
         : mStream(new std::ostringstream)
         , mStatus(status)
      {
         /* Do nothing. */ ;
      }

      StatusStreamer(const StatusStreamer& rhs)
         : mStream(rhs.mStream)
         , mStatus(rhs.mStatus)
      {
         /* Do nothing. */ ;
      }

      ~StatusStreamer()
      {
         if(mStream)
         {
            mStatus->writeStatusMsg(mStream->str());
         }

         mStatus = NULL;
      }

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
   std::vector<status_func_t>  mOutputFuncs;    /**< List of listeners to pass messages. */
};

}  // namespace inf

#define IOV_STATUS (*(inf::Status::instance()->getStreamer().stream()))

#endif
