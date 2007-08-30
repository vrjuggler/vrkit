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


