// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_WANDINTERFACE_H_
#define _INF_WANDINTERFACE_H_

#include <IOV/Config.h>

#include <string>
#include <vector>

#include <jccl/Config/ConfigElementPtr.h>
#include <gadget/Type/DigitalInterface.h>
#include <gadget/Type/PositionInterface.h>
#include <gadget/Type/AnalogInterface.h>

#include <IOV/ViewerPtr.h>
#include <IOV/WandInterfacePtr.h>
#include <IOV/DeviceInterface.h>


namespace inf
{

/**
 * Simple, configurable interface for a wand.
 */
class IOV_CLASS_API WandInterface : public DeviceInterface
{
public:
   WandInterface();

   /**
    * Configures this wand interface.
    *
    * @post This wand interface is configured and ready to use. If an
    *       appropriate configuration element is provided, it will be used.
    *       If it is invalid, the default configuration will be used as the
    *       fallback. Otehrwise, if no appropriate configuration element is
    *       provided, then the default configuration will be used.
    *
    * @see configure()
    * @see configureDefault()
    */
   void init(inf::ViewerPtr viewer);

   /** Returns the position interface for the wand. */
   gadget::PositionInterface& getWandPos();

   /**
    * Returns the digital interface to the given button number.
    * If \p buttonNum == -1 or is out of range, then return a dummy interface
    * that always returns \c gadget::Digital::OFF.
    */
   gadget::DigitalInterface& getButton(const int buttonNum);

   /**
    * Returns the analog interface to the given analog number.
    * If \p analogNum == -1 or is out of range, then return a dummy interface
    * that always returns default value.
    */
   gadget::AnalogInterface& getAnalog(const int analogNum);

private:
   /** @name Configuration Aspects */
   //@{
   /**
    * Returns the name of the config element type used to configure this
    * object type.
    *
    * @since 0.39.0
    */
   static std::string getElementType()
   {
      return "iov_wand_interface";
   }

   /**
    * Configures this object.
    *
    * @post \c mWandInterface is initialized using the configured position
    *       proxy name or alias. \c mButtonInterfaces contains the number of
    *       configured gadget::DigitalInterface objects, all of which are
    *       initialized using the configured names. \c mAnalogInterfaces
    *       contains the number of configured gadget::DigitalInterface
    *       objects, all of which are initialized using the configured names.
    *
    * @throw inf::Exception Thrown if the configuration given is invalid.
    *
    * @since 0.39.0
    */
   void configure(jccl::ConfigElementPtr elt);

   /**
    * Configures this object with the default (static, backwards compatible)
    * settings. These settings are the following:
    *
    *  - \c mWandInterface is initialized using "VJWand"
    *  - Six buttons are configured in \c mButtonInterfaces using "VJButton0"
    *    through "VJButton1"
    *  - Four analog axes are configured in \c mAnalogInterfaces using
    *    "VJAnalog0" through "VJAnalaog1"
    */
   void configureDefault();
   //@}

   gadget::PositionInterface              mWandInterface;
   std::vector<gadget::DigitalInterface>  mButtonInterfaces;   /**< References to VRJ buttons. */
   std::vector<gadget::AnalogInterface>   mAnalogInterfaces;   /**< References to VRJ analogs. */

   gadget::DigitalInterface               mDummyDigital; /**< Dummy to return for digital. */
   gadget::AnalogInterface                mDummyAnalog;  /**< Dummy to return for analog. */
};

}


#endif
