#ifndef INFI_WANDINTERFACE_H
#define INFI_WANDINTERFACE_H

#include <IOV/DeviceInterface.h>
#include <gadget/Type/DigitalInterface.h>
#include <gadget/Type/PositionInterface.h>
#include <vector>

#include <IOV/WandInterfacePtr.h>


namespace inf
{

/** Simple stupid interface for a wand.
 *
 * Just uses VJWand and VJButton0..4
 */
class WandInterface : public DeviceInterface
{
public:
   WandInterface()
   { init(); }

   void init();

   /** Return the position of the wand. */
   gadget::PositionInterface& getWandPos();

   /** Return the interface to the given button number.
    * If button number == -1 or is out of range, then
    * return a dummy interface that always returns OFF.
    */
   gadget::DigitalInterface& getButton(int buttonNum);

protected:
   gadget::PositionInterface              mWandInterface;
   std::vector<gadget::DigitalInterface>  mButtonInterfaces;
   gadget::DigitalInterface               mDummyDigital;
};


}

#endif
