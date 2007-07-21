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

} // namespace inf
