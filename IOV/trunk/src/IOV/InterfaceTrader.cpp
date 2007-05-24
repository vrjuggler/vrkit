// Copyright (C) Infiscape Corporation 2005-2007

#include <IOV/WandInterface.h>
#include <IOV/InterfaceTrader.h>


namespace inf
{

WandInterfacePtr InterfaceTrader::getWandInterface()
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

} // namespace inf
