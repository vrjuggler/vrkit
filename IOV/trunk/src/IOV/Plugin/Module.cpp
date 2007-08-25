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

#include <cstdlib>
#include <sstream>

#include <IOV/Util/Exceptions.h>
#include <IOV/Plugin/Module.h>


namespace inf
{

namespace plugin
{

Module::Module(vpr::LibraryPtr module)
   : mModule(module)
{
   /* Do nothing. */ ;
}

bool Module::hasSymbol(const std::string& name) const
{
   return mModule->findSymbol(name) != NULL;
}

void* Module::getSymbol(const std::string& name) const
{
   void* symbol = mModule->findSymbol(name);

   if ( NULL == symbol )
   {
      std::ostringstream msg_stream;
      msg_stream << "Plug-in module '" << mModule->getName()
                 << "' has no entry point function named " << name;
      throw inf::PluginInterfaceException(msg_stream.str(), IOV_LOCATION);
   }

   return symbol;
}

}

}
