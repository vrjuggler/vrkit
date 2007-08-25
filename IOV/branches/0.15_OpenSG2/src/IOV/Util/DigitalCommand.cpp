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

#include <algorithm>
#include <numeric>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <IOV/WandInterface.h>
#include <IOV/Util/DigitalCommand.h>


namespace inf
{

DigitalCommand::DigitalCommand()
   : mButtonVec(1, -1)
{
   /* Do nothing. */ ;
}

DigitalCommand::DigitalCommand(const std::string& buttonString)
   : mButtonVec(1, -1)
{
   configButtons(buttonString);
}

// buttonString is passed by copy on purpose so that boost::trim() can be used
// on it.
void DigitalCommand::configButtons(std::string buttonString)
{
   // Clean up buttonString before we try to use it.
   boost::trim(buttonString);

   // Extract the comma-separated values from buttonString and put them into
   // btn_strings.
   std::vector<std::string> btn_strings;
   boost::split(btn_strings, buttonString, boost::is_any_of(", "));

   // Convert all the string-ified integers in btn_strings into integers and
   // store them in mButtonVec.
   mButtonVec.resize(btn_strings.size());
   std::transform(
      btn_strings.begin(), btn_strings.end(), mButtonVec.begin(),
      boost::function<int (const std::string&)>(boost::lexical_cast<int, std::string>)
   );
}

bool DigitalCommand::isConfigured() const
{
   return mButtonVec[0] != -1;
}

bool DigitalCommand::test(inf::WandInterfacePtr wandIf,
                          const gadget::Digital::State testState)
{
   if ( mButtonVec.empty() || ! isConfigured() )
   {
      return false;
   }
   else
   {
      return std::accumulate(mButtonVec.begin(), mButtonVec.end(), true,
                             boost::bind(&DigitalCommand::accumulateState,
                                         wandIf, testState, _1, _2));
   }
}

bool DigitalCommand::accumulateState(inf::WandInterfacePtr wandIf,
                                     const gadget::Digital::State testState,
                                     const bool accumState, const int btn)
{
   return accumState && wandIf->getButton(btn)->getData() == testState;
}

}
