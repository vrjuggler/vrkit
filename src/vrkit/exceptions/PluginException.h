// vrkit is (C) Copyright 2005-2008
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

#ifndef _VRKIT_PLUGIN_EXCEPTION_H_
#define _VRKIT_PLUGIN_EXCEPTION_H_

#include <vrkit/Config.h>

#include <vrkit/Exception.h>


namespace vrkit
{

/** \class PluginException PluginException.h vrkit/exceptions/PluginException.h
 *
 * Base exception type for plug-in error handling.
 */
class VRKIT_CLASS_API PluginException : public Exception
{
public:
   PluginException(const std::string& msg, const std::string& location = "")
      throw ();

   virtual ~PluginException() throw ();

   std::string getExceptionName()
   {
      return "vrkit::PluginException";
   }
};

}


#endif /* _VRKIT_PLUGIN_EXCEPTION_H_ */
