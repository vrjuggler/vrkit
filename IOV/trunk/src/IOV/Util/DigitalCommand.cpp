// Copyright (C) Infiscape Corporation 2005-2007

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
      boost::bind(boost::lexical_cast<int, std::string>, _1)
   );
}

bool DigitalCommand::isConfigured() const
{
   return mButtonVec[0] != -1;
}

#if defined(_MSC_VER) && _MSC_VER >= 1400
struct CallWrapper
{
   CallWrapper(inf::WandInterfacePtr wandIf,
               const gadget::Digital::State testState)
      : mWandIf(wandIf)
      , mTestState(testState)
   {
   }

   bool operator()(const bool b, const int i)
   {
      return DigitalCommand::accumulateState(mWandIf, mTestState, b, i);
   }

   inf::WandInterfacePtr mWandIf;
   const gadget::Digital::State mTestState;
};
#endif

bool DigitalCommand::test(inf::WandInterfacePtr wandIf,
                          const gadget::Digital::State testState)
{
   if ( mButtonVec.empty() || ! isConfigured() )
   {
      return false;
   }
   else
   {
#if defined(_MSC_VER) && _MSC_VER >= 1400
      // For some reason, the use of boost::bind() does not compile when
      // building with Visual C++ 8.0--at least against Boost 1.33.1.
      CallWrapper c(wandIf, testState);
      return std::accumulate(mButtonVec.begin(), mButtonVec.end(), true,
                             c);
#else
      return std::accumulate(mButtonVec.begin(), mButtonVec.end(), true,
                             boost::bind(&DigitalCommand::accumulateState,
                                         wandIf, testState, _1, _2));
#endif
   }
}

bool DigitalCommand::accumulateState(inf::WandInterfacePtr wandIf,
                                     const gadget::Digital::State testState,
                                     const bool accumState, const int btn)
{
   return accumState && wandIf->getButton(btn)->getData() == testState;
}

}
