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

#ifndef _VRKIT_EXCEPTION_H_
#define _VRKIT_EXCEPTION_H_

#include <vrkit/Config.h>

#include <stdexcept>
#include <string>

//#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/stringize.hpp>

#define VRKIT_LOCATION std::string(__FILE__) + std::string(":") + \
                       std::string(BOOST_PP_STRINGIZE(__LINE__))

// Example:
//  throw vrkit::Exception("ObjectProxy not ready in Node::objProxy_checked",
//                         VRKIT_LOCATION);

// Exception areas
// - I/O loading/saving issues
// - Property access errors
// - Invalid data type errors


namespace vrkit
{

/** \class Exception Exception.h vrkit/Exception.h
 *
 * Base exception type for all vrkit exceptions.
 */
class VRKIT_CLASS_API Exception : public std::runtime_error
{
public:
   Exception(const std::string& desc, const std::string& location) throw ();

   virtual ~Exception() throw ();

   virtual const char* what() const throw ();

   virtual std::string getExceptionName() const;

   const std::string& getDescription() const;

   void setDescription(const std::string& desc);

   /** Slightly longer description */
   virtual std::string getExtendedDescription() const;

   /** Description with everything we know */
   virtual std::string getFullDescription() const;

protected:
   std::string mDescription;
   std::string mLocation;
   std::string mStackTrace;

   /** Temporary string to return as char* where needed. */
   mutable std::string mFullDesc;
};

}


#endif /* _VRKIT_EXCEPTION_H_ */
