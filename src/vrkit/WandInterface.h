// vrkit is (C) Copyright 2005-2008
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

#ifndef _VRKIT_WAND_INTERFACE_H_
#define _VRKIT_WAND_INTERFACE_H_

#include <vrkit/Config.h>

#include <string>
#include <vector>

#include <jccl/Config/ConfigElementPtr.h>
#include <gadget/Type/DigitalInterface.h>
#include <gadget/Type/PositionInterface.h>
#include <gadget/Type/AnalogInterface.h>

#include <vrkit/ViewerPtr.h>
#include <vrkit/WandInterfacePtr.h>
#include <vrkit/DeviceInterface.h>


namespace vrkit
{

/** \class WandInterface WandInterface.h vrkit/WandInterface.h
 *
 * Simple, configurable interface for a wand.
 */
class VRKIT_CLASS_API WandInterface : public DeviceInterface
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
   void init(ViewerPtr viewer);

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
      return "vrkit_wand_interface";
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
    * @throw vrkit::Exception Thrown if the configuration given is invalid.
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


#endif /* _VRKIT_WAND_INTERFACE_H_ */
