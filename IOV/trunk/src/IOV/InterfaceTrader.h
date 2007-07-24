// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_INTERFACE_TRADER_H
#define _INF_INTERFACE_TRADER_H

#include <IOV/Config.h>
#include <IOV/WandInterfacePtr.h>

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
class IOV_CLASS_API InterfaceTrader
{
public:
   /**
    * Returns a shared pointer to a wand interface.
    *
    * @post A valid WandInterfacePtr is returned.
    */
   WandInterfacePtr getWandInterface();

protected:
   WandInterfacePtr mWandInterface;
};

} // namespace inf

#endif
