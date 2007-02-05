// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_DIGITAL_COMMAND_H_
#define _INF_DIGITAL_COMMAND_H_

#include <IOV/Config.h>

#include <string>
#include <vector>

#include <gadget/Type/Digital.h>
#include <IOV/WandInterfacePtr.h>


namespace inf
{

/**
 * This class represents a combination of one or more digital buttons
 * (identified by an integer index corresponding to a digital button defined by
 * inf::WandInterface) as a "command." The status of the buttons can be tested
 * to determine if this command is to be activated.
 *
 * @since 0.15.0
 *
 * @see inf::WandInterface
 */
class IOV_CLASS_API DigitalCommand
{
public:
   /**
    * Constructs an unconfigured digital command object.
    *
    * @see configButtons()
    */
   DigitalCommand();

   /**
    * Constructs a digital command and configures it using the given string
    * of button identifiers.
    *
    * @param buttonString The string that describes the buttons to use. The
    *                     string must be a comma-separated list of integer
    *                     identifiers. The integer identifiers must be in
    *                     the range buttons supported by inf::WandInterface.
    *
    * @see configButtons()
    */
   DigitalCommand(const std::string& buttonString);

   /**
    * Configures the buttons to be utilized by this digital command.
    *
    * @post \c mButtonVec is populated with the integers extracted from
    *       \p buttonString.
    *
    * @param buttonString The string that describes the buttons to use. The
    *                     string must be a comma-separated list of integer
    *                     identifiers. The integer identifiers must be in
    *                     the range buttons supported by inf::WandInterface.
    *
    * @see inf::WandInterface
    */
   void configButtons(std::string buttonString);

   /**
    * Determines whether this digital command object has had its buttons
    * configured. If this command object is not configured, then test() will
    * return false.
    *
    * @see configButtons()
    * @see test()
    */
   bool isConfigured() const;

   /**
    * Determines whether this digital command is in the given state.
    *
    * @param wandIf
    * @param testState The state of a gadget::Digital object to be tested
    *                  against the current state of this digital command.
    */
   bool test(inf::WandInterfacePtr wandIf,
             const gadget::Digital::State testState);

   /**
    * Replaces the collection of buttons in this digital command with the
    * given vector of button identifiers.
    *
    * @pre The identifiers given in \p buttons must all be valid with respect
    *      to the digital buttons provided by inf::WandInterface.
    * @post \c mButtonVec is replaced by the conents of \p buttons.
    *
    * @param buttons A vector of integer identifiers corresponding to the
    *                digital buttons provided by inf::WandInterface.
    */
   void setButtons(const std::vector<int>& buttons)
   {
      mButtonVec = buttons;
   }

   /**
    * Returns the button identifiers currently in use by this digital command.
    */
   const std::vector<int>& getButtons() const
   {
      return mButtonVec;
   }

private:
   /**
    * Performs an accumulated test of all the digital buttons in use by this
    * digital command to determine if this command is active. For this command
    * to be active, all the buttons in \c mButtonVec must be in the given
    * state.
    *
    * @pre \p btn is a valid digital button identifier for \p wandIf.
    *
    * @param wandIf     The wand interface object holding the actual digital
    *                   interfaces that will be queried.
    * @param testState  The desired digital button state that will be tested.
    * @param accumState The current accumulated state of the buttons that are
    *                   in \p testState.
    * @param btn        The button whose state will be tested and accumulated
    *                   with previous tests.
    *
    * @note This method is designed to be used with std::accumulate() and
    *       really does not make sense to be used on its own.
    */
   static bool accumulateState(inf::WandInterfacePtr wandIf,
                               const gadget::Digital::State testState,
                               const bool accumState, const int btn);

#if defined(_MSC_VER) && _MSC_VER >= 1400
   friend struct CallWrapper;
#endif

   std::vector<int> mButtonVec;
};

}


#endif /* _INF_DIGITAL+_COMMAND_H_ */
