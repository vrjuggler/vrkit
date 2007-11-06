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

#include <vrkit/Config.h>

#include <sstream>
#include <algorithm>

#include <vrkit/Exception.h>
#include <vrkit/plugin/data/ValueDesc.h>
#include <vrkit/plugin/data/TypeDesc.h>


namespace vrkit
{

namespace plugin
{

namespace data
{

TypeDesc::TypeDesc(cppdom::NodePtr decl)
{
   cppdom::Attribute id_attr = decl->getAttribute(tokens::id_attr);
   mID = vpr::GUID(id_attr.getValue<std::string>());

   cppdom::NodeList& children = decl->getChildren();
   typedef cppdom::NodeList::iterator iter_type;

   for ( iter_type c = children.begin(); c != children.end(); ++c )
   {
      const std::string node_name((*c)->getName());

      cppdom::Attribute name_attr  = (*c)->getAttribute(tokens::name_attr);
      const std::string value_name = name_attr.getValue<std::string>();

      if ( node_name == tokens::datum_elt )
      {
         cppdom::Attribute type_attr = (*c)->getAttribute(tokens::type_attr);
         const std::string type_name = type_attr.getValue<std::string>();

         if ( type_name == tokens::bool_value )
         {
            mValueDescs.push_back(
               ValueDesc(value_name, getInitValue<bool>(*c))
            );
         }
         else if ( type_name == tokens::uint8_value )
         {
            mValueDescs.push_back(
               ValueDesc(value_name, getInitValue<vpr::Uint8>(*c))
            );
         }
         else if ( type_name == tokens::int8_value )
         {
            mValueDescs.push_back(
               ValueDesc(value_name, getInitValue<vpr::Int8>(*c))
            );
         }
         else if ( type_name == tokens::uint16_value )
         {
            mValueDescs.push_back(
               ValueDesc(value_name, getInitValue<vpr::Uint16>(*c))
            );
         }
         else if ( type_name == tokens::int16_value )
         {
            mValueDescs.push_back(
               ValueDesc(value_name, getInitValue<vpr::Int16>(*c))
            );
         }
         else if ( type_name == tokens::uint32_value )
         {
            mValueDescs.push_back(
               ValueDesc(value_name, getInitValue<vpr::Uint32>(*c))
            );
         }
         else if ( type_name == tokens::int32_value )
         {
            mValueDescs.push_back(
               ValueDesc(value_name, getInitValue<vpr::Int32>(*c))
            );
         }
         else if ( type_name == tokens::uint64_value )
         {
            mValueDescs.push_back(ValueDesc(value_name,
                                            getInitValue<vpr::Uint64>(*c)));
         }
         else if ( type_name == tokens::int64_value )
         {
            mValueDescs.push_back(ValueDesc(value_name,
                                            getInitValue<vpr::Int64>(*c)));
         }
         else if ( type_name == tokens::float_value )
         {
            mValueDescs.push_back(ValueDesc(value_name,
                                            getInitValue<float>(*c)));
         }
         else if ( type_name == tokens::double_value )
         {
            mValueDescs.push_back(ValueDesc(value_name,
                                            getInitValue<double>(*c)));
         }
         else if ( type_name == tokens::string_value )
         {
            mValueDescs.push_back(ValueDesc(value_name,
                                            getInitValue<std::string>(*c)));
         }
      }
      else if ( node_name == tokens::pointer_elt )
      {
         cppdom::Attribute pointee_attr =
            (*c)->getAttribute(tokens::pointee_type_attr);
         const std::string pointee_type = pointee_attr.getValue<std::string>();
         mValueDescs.push_back(ValueDesc(value_name, ptr_type(0),
                                         vpr::GUID(pointee_type)));
      }
   }
}

const ValueDesc& TypeDesc::getValueDesc(const std::string& name) const
{
   typedef std::vector<ValueDesc>::const_iterator iter_type;
   MemberNamePredicate pred(name);
   iter_type i = std::find_if(mValueDescs.begin(), mValueDescs.end(), pred);

   if ( i == mValueDescs.end() )
   {
      std::ostringstream msg_stream;
      msg_stream << "No matching value named '" << name << "'";
      throw vrkit::Exception(msg_stream.str(), VRKIT_LOCATION);
   }

   return *i;
}

bool TypeDesc::MemberNamePredicate::operator()(const ValueDesc& d)
{
   return d.getName() == mName;
}

}

}

}
