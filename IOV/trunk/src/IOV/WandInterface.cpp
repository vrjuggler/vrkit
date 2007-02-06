// Copyright (C) Infiscape Corporation 2005-2007

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

gadget::AnalogInterface& WandInterface::getAnalog(int analogNum)
{
   if( (analogNum >= 0) &&
       (analogNum < int(mAnalogInterfaces.size())))
   {
      return mAnalogInterfaces[analogNum];
   }
   else
   {
      return mDummyAnalog;
   }
}

void WandInterface::init()
{
   mWandInterface.init("VJWand");

   mButtonInterfaces.resize(6);
   mButtonInterfaces[0].init("VJButton0");
   mButtonInterfaces[1].init("VJButton1");
   mButtonInterfaces[2].init("VJButton2");
   mButtonInterfaces[3].init("VJButton3");
   mButtonInterfaces[4].init("VJButton4");
   mButtonInterfaces[5].init("VJButton5");

   mAnalogInterfaces.resize(4);
   mAnalogInterfaces[0].init("VJAnalog0");
   mAnalogInterfaces[1].init("VJAnalog1");
   mAnalogInterfaces[2].init("VJAnalog2");
   mAnalogInterfaces[3].init("VJAnalog3");
}

}

