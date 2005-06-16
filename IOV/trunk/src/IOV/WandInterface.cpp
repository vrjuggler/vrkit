// Copyright (C) Infiscape Corporation 2005

#include <IOV/WandInterface.h>

namespace inf
{

gadget::PositionInterface& WandInterface::getWandPos()
{ return mWandInterface; }

gadget::DigitalInterface& WandInterface::getButton(int buttonNum)
{
   if( (buttonNum >= 0) &&
       (buttonNum < int(mButtonInterfaces.size())) )
   {
      return mButtonInterfaces[buttonNum];
   }
   else
   {
      return mDummyDigital;
   }
}

void WandInterface::init()
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

