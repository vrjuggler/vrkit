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

#include <sstream>

#include <vpr/Util/Assert.h>
#include <jccl/Config/ConfigElement.h>

#include <IOV/Viewer.h>
#include <IOV/Util/Exceptions.h>

#include <IOV/WandInterface.h>


namespace inf
{

WandInterface::WandInterface()
{
   /* Do nothing. */ ;
}

void WandInterface::init(inf::ViewerPtr viewer)
{
   jccl::ConfigElementPtr cfg_elt =
      viewer->getConfiguration().getConfigElement(getElementType());

   if ( cfg_elt )
   {
      try
      {
         configure(cfg_elt);
      }
      catch (inf::Exception& ex)
      {
         std::cerr << "Configuration of WandInterface failed:\n" << ex.what()
                   << "\nUsing default settings." << std::endl;
         configureDefault();
      }
   }
   else
   {
      configureDefault();
   }
}

gadget::PositionInterface& WandInterface::getWandPos()
{
   return mWandInterface;
}

gadget::DigitalInterface& WandInterface::getButton(const int buttonNum)
{
   if ( buttonNum >= 0 &&
        buttonNum < static_cast<int>(mButtonInterfaces.size()) )
   {
      return mButtonInterfaces[buttonNum];
   }
   else
   {
      return mDummyDigital;
   }
}

gadget::AnalogInterface& WandInterface::getAnalog(const int analogNum)
{
   if ( analogNum >= 0 &&
        analogNum < static_cast<int>(mAnalogInterfaces.size()) )
   {
      return mAnalogInterfaces[analogNum];
   }
   else
   {
      return mDummyAnalog;
   }
}

void WandInterface::configure(jccl::ConfigElementPtr elt)
{
   vprASSERT(elt->getID() == getElementType());

   const unsigned int req_cfg_version(1);

   if ( elt->getVersion() < req_cfg_version )
   {
      std::ostringstream msg;
      msg << "Configuration of WandInterface failed.  Required config "
          << "element version is " << req_cfg_version << ", but element '"
          << elt->getName() << "' is version " << elt->getVersion();
      throw inf::Exception(msg.str(), IOV_LOCATION);
   }

   const std::string pos_name_prop("position_name");
   const std::string digital_name_prop("digital_name");
   const std::string analog_name_prop("analog_name");

   const std::string wand_name = elt->getProperty<std::string>(pos_name_prop);

   if ( wand_name.empty() )
   {
      throw inf::Exception("Empty wand name is not allowed", IOV_LOCATION);
   }

   mWandInterface.init(wand_name);

   const unsigned int num_digitals(elt->getNum(digital_name_prop));

   if ( num_digitals > 0 )
   {
      mButtonInterfaces.resize(num_digitals);

      for ( unsigned int d = 0; d < num_digitals; ++d )
      {
         const std::string digital_name =
            elt->getProperty<std::string>(digital_name_prop, d);

         if ( digital_name.empty() )
         {
            mButtonInterfaces.clear();

            std::ostringstream msg_stream;
            msg_stream << "Empty digital name (index " << d
                       << ") is not allowed";
            throw inf::Exception(msg_stream.str(), IOV_LOCATION);
         }

         mButtonInterfaces[d].init(digital_name);
      }
   }

   const unsigned int num_analogs(elt->getNum(analog_name_prop));

   if ( num_analogs > 0 )
   {
      mAnalogInterfaces.resize(num_analogs);

      for ( unsigned int a = 0; a < num_analogs; ++a )
      {
         const std::string analog_name =
            elt->getProperty<std::string>(analog_name_prop, a);

         if ( analog_name.empty() )
         {
            mAnalogInterfaces.clear();

            std::ostringstream msg_stream;
            msg_stream << "Empty analog name (index " << a
                       << ") is not allowed";
            throw inf::Exception(msg_stream.str(), IOV_LOCATION);
         }

         mAnalogInterfaces[a].init(analog_name);
      }
   }
}

void WandInterface::configureDefault()
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
