#ifndef WANDINTERFACE_H
#define WANDINTERFACE_H

#include <OpenSG/VRJ/Viewer/DeviceInterface.h>
#include <gadget/Type/DigitalInterface.h>
#include <gadget/Type/PositionInterface.h>
#include <vector>

#include <OpenSG/VRJ/Viewer/WandInterfacePtr.h>


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
   gadget::PositionInterface& getWandPos()
   { return mWandInterface; }

   /** Return the interface to the given button number. */
   gadget::DigitalInterface& getButton(unsigned buttonNum)
   { return mButtonInterfaces[buttonNum]; }

protected:
   gadget::PositionInterface              mWandInterface;
   std::vector<gadget::DigitalInterface>  mButtonInterfaces;
};

inline void WandInterface::init()
{
   mWandInterface.init("VJWand");

   mButtonInterfaces.resize(5);
   mButtonInterfaces[0].init("VJButton0");
   mButtonInterfaces[1].init("VJButton1");
   mButtonInterfaces[2].init("VJButton2");
   mButtonInterfaces[3].init("VJButton3");
   mButtonInterfaces[4].init("VJButton4");
}

}

#endif
