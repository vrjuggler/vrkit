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

#ifndef _VRKIT_PLUGIN_DATA_VALUE_DESC_H_
#define _VRKIT_PLUGIN_DATA_VALUE_DESC_H_

#include <vrkit/Config.h>

#include <string>
#include <boost/any.hpp>

#include <vpr/Util/GUID.h>


namespace vrkit
{

namespace plugin
{

namespace data
{

/**
 * Description of a value (an individual data member) within a dynamically
 * defined plug-in data type. Instances of this class are held by the type
 * description (an instance of vrkit::plugin::data::TypeDesc) for the
 * dynamically defined type.
 *
 * @see vrkit::plugin::data::TypeDesc
 *
 * @since 0.51.1
 */
class VRKIT_CLASS_API ValueDesc
{
public:
   /**
    * Constructor for a data member that is not a pointer type.
    *
    * @param name      The name of this data member.
    * @param initValue The initial value of this data member. The C++ type
    *                  contained by the given boost::any object must match the
    *                  type given in the source XML description.
    */
   ValueDesc(const std::string& name, const boost::any& initValue);

   /**
    * Constructor for a data member that is a pointer to another instance of
    * a dynamically defined data type.
    *
    * @param name        The name of this data member.
    * @param initValue   The initial value of this data member. For this case,
    *                    this value should always be 0.
    * @param pointeeType The unique identifier of the type to which this data
    *                    member is allowed to point.
    */
   ValueDesc(const std::string& name, const boost::any& initValue,
             const vpr::GUID& pointeeType);

   const std::string& getName() const
   {
      return mName;
   }

   const boost::any& getInitValue() const
   {
      return mInitValue;
   }

   const vpr::GUID& getPointeeType() const
   {
      return mPointeeType;
   }

   /**
    * Determines if this data member accepts the given type identifier as a
    * pointee type. If this data member is not for a pointer, the "accepted"
    * pointee type is a null GUID, an invalid type identifier.
    */
   bool acceptsPointee(const vpr::GUID& typeID) const
   {
      return mPointeeType == typeID;
   }

private:
   std::string mName;           /**< The name of this data member */
   boost::any  mInitValue;      /**< The initial value of this data member */

   /**
    * The accepted pointee type of this data member--if this data member is
    * for a pointer. If this data member is not for a pointer, then this will
    * be a null GUID, which is not a valid type identifier.
    */
   vpr::GUID mPointeeType;
};

}

}

}


#endif /* _VRKIT_PLUGIN_DATA_VALUE_DESC_H_ */
