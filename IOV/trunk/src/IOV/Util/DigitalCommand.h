// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_DIGITAL_COMMAND_H_
#define _INF_DIGITAL_COMMAND_H_

#include <IOV/Config.h>

#include <string>
#include <boost/function.hpp>

#include <gadget/Type/Digital.h>
#include <IOV/WandInterfacePtr.h>


namespace inf
{

/**
 * @example "Example of configuring inf::DigitalCommand"
 *
 * The language of inf::DigitalCommand defines a boolean expression. Each
 * digital button is identified by a zero-based integer, and its state is
 * identified by an operator. Gadgeteer digital devices are in one of four
 * states: off (-), on (+), toggle on (^), toggle off (v). Boolean operators
 * are used to combine the button states. There are three such operators:
 * and (&), inclusive or (|), and exclusive or (^). Parentheses can be used
 * for (arbitrarily deep) grouping. The state of a button or a group can be
 * negated using the ! operator.
 *
 * The following are examples of digital command configurations:
 *   - 0-: Button 0 in the off state
 *   - 1+: Butotn 1 in the on state
 *   - 0^ | 2^: Buttons 0 and 2 are in the toggled on state
 *   - !3+: Button 3 is not in the on state
 *   - (1v | 2v) & 3+: Buttons 1 or 2 are in the toggled off state while
 *     button 3 is in the on state
 *   - 0- & !(1+ | 2+): Button 0 is in the off state and neither button 1 nor
 *     button 2 is in the on state
 *   - 3^ ^ 7^: Button 3 or button 7 is in the toggled on state but not both
 *
 * To disable the use of a digital command, configure it using the empty
 * string.
 *
 * The button indexing is based on how the wand interface is configured. By
 * default, the IOV wand interface has 6 buttons, but it can be configured to
 * have any number. It is therefore very important to ensure that the digital
 * device indices used in configuring a digital command are valid with
 * respect to the wand button configuration.
 *
 * @see inf::WandInterface
 */

/**
 * This class represents a combination of one or more digital buttons
 * (identified by an integer index corresponding to a digital button defined
 * by inf::WandInterface) as a "command." A configuration string defines the
 * "command" based on the state of one or more digitial devices. A digital
 * command is in either the on (active) or the off (inactive) state. The state
 * is determined by querying the state of the each of the digital devices
 * associated with the command.
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
    * @see configure()
    */
   DigitalCommand();

   /** @name Configuration */
   //@{
   /**
    * Configures the buttons to be utilized by this digital command.
    *
    * @post \c mButtonVec is populated with the integers extracted from
    *       \p buttonString.
    *
    * @param buttonString The string that describes the button state for
    *                     this digital command. This parameter is passed by
    *                     copy on purpose.
    * @param wandIf       The wand interface from which the buttons will be
    *                     queried.
    *
    * @see inf::WandInterface
    *
    * @throw inf::Exception Thrown if a parse error occurs.
    *
    * @since 0.41.0
    */
   void configure(std::string buttonString, inf::WandInterfacePtr wandIf);

   /**
    * Determines whether this digital command object has had its buttons
    * configured. If this command object is not configured, then test() will
    * return false.
    *
    * @see configure()
    * @see test()
    */
   bool isConfigured() const
   {
      return ! mConfigString.empty() && ! mTestFunc.empty();
   }
   //@}

   /** @name State Test */
   //@{
   /**
    * Determines whether this digital command is in the given state.
    *
    * @pre This digital command was configured successfully.
    *
    * @since 0.41.0
    */
   bool test() const
   {
      return mTestFunc();
   }

   /**
    * A synonym for test().
    *
    * @since 0.41.0
    */
   bool operator()() const
   {
      return test();
   }
   //@}

   /**
    * Returns the original boolean expression in string form of this digital
    * command's configuration.
    *
    * @since 0.41.0
    */
   const std::string& getExpression() const
   {
      return mConfigString;
   }

   /**
    * Provides a human-readable string form of this digital command.
    *
    * @note The intention of this method is to transform the boolean
    *       expression string into something meaningful to the average user.
    *       Doing this is rather non-trivial, but utilizing the abstract
    *       syntax tree in configure() would be a start. At any rate, for now,
    *       it just returns the original boolean expression used to configure
    *       this digital command.
    *
    * @since 0.41.0
    */
   const std::string& toString() const
   {
      return getExpression();
   }

   /**
    * Compares this digital command against the given digital command to
    * determine if they are equivalent. Equivalency is defined as two digital
    * commands that are in the active state at the same time.
    *
    * @since 0.41.0
    */
   bool operator==(const DigitalCommand& rhs) const;

private:
   /**
    * Used for simple state tests such as the case of a disabled digital
    * command. This always returns false.
    *
    * @since 0.41.0
    */
   static bool dummyTest()
   {
      return false;
   }

   std::string              mConfigString;
   boost::function<bool ()> mTestFunc;
};

}


#endif /* _INF_DIGITAL+_COMMAND_H_ */
