#ifndef _INTERFACE_TRADER_H
#define _INTERFACE_TRADER_H

#include <OpenSG/VRJ/Viewer/IOV/DeviceInterface.h>
#include <OpenSG/VRJ/Viewer/IOV/WandInterface.h>


namespace inf
{

/** Class for creating a desired IO interface based on reqs.
* This will be an implementation of the Product Trader pattern (when complete)
*
* The idea is to give this class a description of what is needed
* for an interface, and then have it create it on the fly by
* finding the best match and adapting the current input in the system
* to dynamically provide the needed capabilities.
*/
class InterfaceTrader
{
public:
   /**
    * Returns a shared pointer to a wand interface.
    *
    * @post A valid WandInterfacePtr is returned.
    */
   WandInterfacePtr getWandInterface()
   {
       WandInterfacePtr wand_interface = mWandInterface.lock();

       if ( wand_interface.get() == NULL )
       {
          wand_interface = WandInterfacePtr(new WandInterface);
          mWandInterface = wand_interface;
       }

       // NOTE: Could throw an exception here if we failed to create the wand interface.

       return wand_interface;
   }

protected:
   WandInterfaceWeakPtr mWandInterface;
};

} // namespace inf

#endif
