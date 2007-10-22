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

#ifndef _VRKIT_CONFIGURATION_EXCEPTION_H_
#define _VRKIT_CONFIGURATION_EXCEPTION_H_

#include <vrkit/Config.h>

#include <vrkit/Exception.h>


namespace vrkit
{

/** \class ConfigurationException ConfigurationException.h vrkit/exceptions/ConfigurationException.h
 *
 * Base exception type for configuration error handling.
 *
 * @since 0.50.1
 */
class VRKIT_CLASS_API ConfigurationException : public Exception
{
public:
   ConfigurationException(const std::string& msg,
                          const std::string& location = "") throw ();

   virtual ~ConfigurationException() throw ();

   std::string getExceptionName()
   {
      return "vrkit::ConfigurationException";
   }
};

}


#endif /* _VRKIT_CONFIGURATION_EXCEPTION_H_ */
