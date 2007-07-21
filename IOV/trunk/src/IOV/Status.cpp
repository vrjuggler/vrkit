// Copyright (C) Infiscape Corporation 2005-2007

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
