// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_WANDINTERFACE_H
#define _INF_WANDINTERFACE_H

#include <IOV/Config.h>
#include <vector>
#include <gadget/Type/DigitalInterface.h>
#include <gadget/Type/PositionInterface.h>
#include <gadget/Type/AnalogInterface.h>

#include <IOV/WandInterfacePtr.h>
#include <IOV/DeviceInterface.h>


namespace inf
{

/**
 * Simple interface for a wand. This depends on VJWand, VJButton[0..5], and
 * VJAnalog[0..3] being defined in the VR Juggler configuration.
 */
class IOV_CLASS_API WandInterface : public DeviceInterface
{
public:
   WandInterface()
   {
      init();
   }

   void init();

   /** Return the position of the wand. */
   gadget::PositionInterface& getWandPos();

   /** Return the interface to the given button number.
    * If button number == -1 or is out of range, then
    * return a dummy interface that always returns OFF.
    */
   gadget::DigitalInterface& getButton(int buttonNum);

   /** Return the interface to the given analog number.
    * If analog number == -1 or is out of range, then
    * return a dummy interface that always returns default value.
    */
   gadget::AnalogInterface& getAnalog(int analogNum);

protected:
   gadget::PositionInterface              mWandInterface;
   std::vector<gadget::DigitalInterface>  mButtonInterfaces;   /**< References to VRJ buttons. */
   std::vector<gadget::AnalogInterface>   mAnalogInterfaces;   /**< References to VRJ analogs. */

   gadget::DigitalInterface               mDummyDigital; /**< Dummy to return for digital. */
   gadget::AnalogInterface                mDummyAnalog;  /**< Dummy to return for analog. */
};


}

#endif
