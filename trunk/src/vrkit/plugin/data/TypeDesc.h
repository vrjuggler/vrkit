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

#ifndef _VRKIT_PLUGIN_DATA_TYPE_DESC_H_
#define _VRKIT_PLUGIN_DATA_TYPE_DESC_H_

#include <vrkit/Config.h>

#include <string>
#include <vector>
#include <boost/algorithm/string/case_conv.hpp>

#include <cppdom/cppdom.h>

#include <vpr/vprTypes.h>
#include <vpr/Util/GUID.h>

#include <vrkit/plugin/data/ValueDesc.h>


namespace vrkit
{

namespace plugin
{

namespace data
{

namespace tokens
{

const std::string datum_elt("datum");
const std::string pointer_elt("pointer");

const std::string name_attr("name");
const std::string id_attr("id");
const std::string type_attr("type");
const std::string init_value_attr("init_value");
const std::string pointee_type_attr("pointee_type");

const std::string bool_value("bool");
const std::string uint8_value("uint8");
const std::string int8_value("int8");
const std::string uint16_value("uint16");
const std::string int16_value("int16");
const std::string uint32_value("uint32");
const std::string int32_value("int32");
const std::string uint64_value("uint64");
const std::string int64_value("int64");
const std::string float_value("float");
const std::string double_value("double");
const std::string string_value("string");

}

/**
 * Description of dynamically defined data types. Instances of this class are
 * constructed from an XML description that spells out the structure of each
 * type. Once constructed, an instance of this type is composed of one or
 * more vrkit::plugin::data::ValueDesc objects.
 *
 * @see vrkit::plugin::Data
 * @see vrkit::plugin::data::ValueDesc
 *
 * @since 0.51.1
 */
class VRKIT_CLASS_API TypeDesc
{
public:
   /**
    * Type for pointers pointer values.
    */
   typedef vpr::Uint64 ptr_type;

   /**
    * Constructs the type description as defined by the given XML tree.
    */
   TypeDesc(cppdom::NodePtr decl);

   /**
    * Returns the unique identifier for this type.
    */
   const vpr::GUID& getID() const
   {
      return mID;
   }

   /** @name Value Declaration Access */
   //@{
   typedef std::vector<ValueDesc>::iterator iterator;
   typedef std::vector<ValueDesc>::const_iterator const_iterator;

   iterator begin()
   {
      return mValueDescs.begin();
   }

   const_iterator begin() const
   {
      return mValueDescs.begin();
   }

   iterator end()
   {
      return mValueDescs.end();
   }

   const_iterator end() const
   {
      return mValueDescs.end();
   }
   //@}

   /**
    * Returns the value description for the named member of this type
    * description.
    *
    * @throw vrkit::Exception
    *           Thrown if \p name does not match the name of any of the
    *           value descriptions held by this type descrption.
    */
   const ValueDesc& getValueDesc(const std::string& name) const;

private:
   /**
    * Simple predicate for use with STL algorithms when it is necessary to
    * find an instance of vrkit::plugin::data::ValueDesc based on its name.
    */
   struct MemberNamePredicate
   {
      MemberNamePredicate(const std::string& name)
         : mName(name)
      {
         /* Do nothing. */ ;
      }

      bool operator()(const ValueDesc& d);

      std::string mName;
   };

   /**
    * Returns the initial value for a data member based on the given XML
    * description. If the XML description includes the initial value, that is
    * what is returned. Otherwise, the value returned by the default
    * constructor for the type is returned to the caller.
    */
   template<typename T>
   T getInitValue(cppdom::NodePtr datumNode)
   {
      // This gives us a way to initialize fundamental data types to 0 and
      // class types (such as std::string) to their proper default.
      T init_value = T();

      if ( datumNode->hasAttribute(tokens::init_value_attr) )
      {
         init_value =
            datumNode->getAttribute(tokens::init_value_attr).template getValue<T>();
      }

      return init_value;
   }

   vpr::GUID mID;

   std::vector<ValueDesc> mValueDescs;
};

/**
 * Specialization of TypeDesc::getInitValue<T>() for bool. This allows users
 * to set the initial value for a boolean data member in XML using the string
 * values "true", "1", "false", or "0". All other values are translated into
 * the \c true value.
 */
template<>
inline bool TypeDesc::getInitValue(cppdom::NodePtr datumNode)
{
   bool init_value(false);

   if ( datumNode->hasAttribute(tokens::init_value_attr) )
   {
      const std::string value_str =
         boost::to_lower_copy(
            datumNode->getAttribute(tokens::init_value_attr).getValue<std::string>()
         );

      // Values of "false" or "0" translate into a boolean false value. All
      // other values  translate to true.
      init_value = (value_str != "false" && value_str != "0");
   }

   return init_value;
}

}

}

}


#endif /* _VRKIT_PLUGIN_DATA_TYPE_DESC_H_ */
