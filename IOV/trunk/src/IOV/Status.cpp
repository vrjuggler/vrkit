#include <IOV/Status.h>


namespace
{

void write_to_cout(const std::string &msg)
{
   std::cout << msg << std::flush;
}

}  // namespace

namespace inf
{

vprSingletonImp(Status);


Status::Status()
{
   addOutputter(write_to_cout);
}

void Status::writeStatusMsg(const std::string& msg)
{
   for(unsigned i=0; i<mOutputFuncs.size(); ++i)
   {
      if(!mOutputFuncs[i].empty())
      {
         (mOutputFuncs[i])(msg);
      }
   }
}

void Status::addOutputter(status_func_t newFunc)
{
   mOutputFuncs.push_back(newFunc);
}

} // namespace inf


