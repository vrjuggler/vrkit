// vrkit is (C) Copyright 2005-2007
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

#ifndef _INTERFACE_TRADER_H
#define _INTERFACE_TRADER_H

#include <IOV/DeviceInterface.h>
#include <IOV/WandInterface.h>


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
