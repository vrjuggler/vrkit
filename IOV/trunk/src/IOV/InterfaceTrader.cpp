// Copyright (C) Infiscape Corporation 2005-2007

#include <IOV/WandInterface.h>
#include <IOV/InterfaceTrader.h>

namespace inf
{

WandInterfacePtr InterfaceTrader::getWandInterface()
{
   if (NULL == mWandInterface.get())
   {
      mWandInterface = WandInterfacePtr(new WandInterface);
   }
   return mWandInterface;
}

} // namespace inf
