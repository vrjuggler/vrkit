// vrkit is (C) Copyright 2005-2011
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

#ifndef _VRKIT_PLUGIN_INTERFACE_EXCEPTION_H_
#define _VRKIT_PLUGIN_INTERFACE_EXCEPTION_H_

#include <vrkit/Config.h>

#include <vrkit/exceptions/PluginException.h>


namespace vrkit
{

/** \class PluginInterfaceException PluginInterfaceException.h vrkit/exceptions/PluginInterfaceException.h
 *
 */
class VRKIT_CLASS_API PluginInterfaceException : public PluginException
{
public:
   PluginInterfaceException(const std::string& msg,
                            const std::string& location = "") throw ();

   virtual ~PluginInterfaceException() throw ();

   std::string getExceptionName()
   {
      return "vrkit::PluginInterfaceException";
   }
};

}


#endif /* _VRKIT_PLUGIN_INTERFACE_EXCEPTION_H_ */
