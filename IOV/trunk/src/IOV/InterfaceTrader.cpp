// Copyright (C) Infiscape Corporation 2005-2007

#include <IOV/WandInterface.h>
#include <IOV/InterfaceTrader.h>

namespace inf
{

void InterfaceTrader::init(inf::ViewerPtr viewer)
{
   mWandInterface = WandInterfacePtr(new WandInterface);
   mWandInterface->init(viewer);
}

WandInterfacePtr InterfaceTrader::getWandInterface()
{
   return mWandInterface;
}

} // namespace inf
