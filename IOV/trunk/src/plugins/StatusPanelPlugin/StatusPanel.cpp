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

#include <vector>
#include <algorithm>
#include <boost/concept_check.hpp>

#include "StatusPanel.h"


namespace inf
{

StatusPanel::StatusPanel()
{
   mHeaderTitle = "Header";
   mCenterTitle = "Center";
   mBottomTitle = "Status";

   mHeaderText  = "Header\nText";

   mStatusHistorySize = 20;
}

void StatusPanel::setHeaderTitle(const std::string& txt)
{
   mHeaderTitle = txt;
   mChangeSignal();
}

void StatusPanel::setCenterTitle(const std::string& txt)
{
   mCenterTitle = txt;
   mChangeSignal();
}

void StatusPanel::setBottomTitle(const std::string& txt)
{
   mBottomTitle = txt;
   mChangeSignal();
}


void StatusPanel::setHeaderText(const std::string& header)
{
   mHeaderText = header;
   mChangeSignal();
}

void StatusPanel::setControlText(const std::string& cmdText,
                                 const std::string& desc)
{
   mCenterText[cmdText].clear();
   mCenterText[cmdText].push_back(desc);
   mChangeSignal();
}

void StatusPanel::addControlText(const std::string& cmdText,
                                 const std::string& desc,
                                 const unsigned int)
{
   // XXX: I don't know how to make a robust priority queue that allows
   // iteration.
   mCenterText[cmdText].push_back(desc);
   mChangeSignal();
}

void StatusPanel::removeControlText(const std::string& cmdText,
                                    const std::string& desc)
{
   std::vector<std::string>::iterator i;
   std::vector<std::string>& vec = mCenterText[cmdText];
   vec.erase(std::remove(vec.begin(), vec.end(), desc), vec.end());
}

void StatusPanel::addStatusMessage(const std::string& msg)
{
   mStatusLines.push_front(msg);

   // Shrink status lines to fit.
   while(mStatusLines.size() > mStatusHistorySize)
   {
      mStatusLines.pop_back();
   }
   mChangeSignal();
}

void StatusPanel::setStatusHistorySize(const unsigned int size)
{
   mStatusHistorySize = size;

   // Shrink status lines to fit.
   while ( mStatusLines.size() > mStatusHistorySize )
   {
      mStatusLines.pop_back();
   }

   mChangeSignal();
}

} // namespace inf
