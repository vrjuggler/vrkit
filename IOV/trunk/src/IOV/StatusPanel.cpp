// Copyright (C) Infiscape Corporation 2005-2007

#include <IOV/StatusPanel.h>

#include <vector>
#include <algorithm>
#include <boost/concept_check.hpp>


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

void StatusPanel::setControlTexts(const std::vector<int>& cmds,
                                 const std::string& desc)
{
   mCenterText[cmds].clear();
   mCenterText[cmds].push_back(desc);
   mChangeSignal();
}

void StatusPanel::addControlTexts(const std::vector<int>& cmds,
                                 const std::string& desc,
                                 const unsigned int priority)
{
   // XXX: I don't know how to make a robust priority queue that allows
   // iteration.
   boost::ignore_unused_variable_warning(priority);
   mCenterText[cmds].push_back(desc);
   mChangeSignal();
}

void StatusPanel::removeControlTexts(const std::vector<int>& cmds,
                                    const std::string& desc)
{
   std::vector<std::string>::iterator i;
   std::vector<std::string>& vec = mCenterText[cmds];
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

