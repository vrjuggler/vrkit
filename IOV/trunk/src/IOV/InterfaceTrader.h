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

#ifndef _INF_INTERFACE_TRADER_H_
#define _INF_INTERFACE_TRADER_H_

#include <IOV/Config.h>

#include <IOV/WandInterfacePtr.h>
#include <IOV/ViewerPtr.h>


namespace inf
{

/**
 * Class for creating a desired IO interface based on reqs.
 * This will be an implementation of the Product Trader pattern (when
 * complete).
 *
 * The idea is to give this class a description of what is needed
 * for an interface, and then have it create it on the fly by
 * finding the best match and adapting the current input in the system
 * to dynamically provide the needed capabilities.
 */
class IOV_CLASS_API InterfaceTrader
{
public:
   void init(inf::ViewerPtr viewer);

   /**
    * Returns a shared pointer to a wand interface.
    */
   WandInterfacePtr getWandInterface();

private:
   WandInterfacePtr mWandInterface;
};

} // namespace inf


#endif
