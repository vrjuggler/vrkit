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

#ifndef _VRKIT_INTERFACE_TRADER_H_
#define _VRKIT_INTERFACE_TRADER_H_

#include <vrkit/Config.h>

#include <vrkit/WandInterfacePtr.h>
#include <vrkit/ViewerPtr.h>


namespace vrkit
{

/**
 * Class for creating a desired IO interface based on requirements. This will
 * be an implementation of the Product Trader pattern (when complete).
 *
 * The idea is to give this class a description of what is needed for an
 * interface so that it can create one on the fly. The Interface Trader does
 * so by finding the best match and adapting the current input device(s)
 * available to provide the needed capabilities dynamically.
 */
class VRKIT_CLASS_API InterfaceTrader
{
public:
   void init(ViewerPtr viewer);

   /**
    * Returns a shared pointer to a wand interface.
    */
   WandInterfacePtr getWandInterface();

private:
   WandInterfacePtr mWandInterface;
};

} // namespace vrkit


#endif /* _VRKIT_INTERFACE_TRADER_H_ */
