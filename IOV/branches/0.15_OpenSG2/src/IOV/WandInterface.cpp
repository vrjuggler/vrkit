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

