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

#include <iostream>
#include <sstream>
#include <iterator>
#include <algorithm>

#include <vrkit/plugin/Info.h>


namespace vrkit
{

namespace plugin
{

std::string Info::sSeparator(":");

Info::Info(const std::string& nameSpace, const std::string& shortName,
           const Info::version_type& version, const std::string& qualifier,
           const std::vector<std::string>& dependencies)
   : mNamespace(nameSpace)
   , mShortName(shortName)
   , mVersion(version)
   , mQualifier(qualifier)
   , mDependencies(dependencies)
{
   mName     = buildNamespaceName(mNamespace, mShortName);
   mFullName = buildFullName(mNamespace, mShortName, mVersion, mQualifier);
}

inline bool inDeps(const std::vector<std::string>& deps,
                   const std::string& name)
{
   return std::find(deps.begin(), deps.end(), name) != deps.end();
}

bool Info::dependsOn(const Info& otherInfo) const
{
   return inDeps(mDependencies, otherInfo.getFullName()) ||
          inDeps(mDependencies, otherInfo.getName());
}

std::string Info::buildFullName(const std::string& nameSpace,
                                const std::string& shortName,
                                const Info::version_type& version,
                                const std::string& qualifier)
{
   std::string full_name(buildNamespaceName(nameSpace, shortName));

   // If a qualifier was specified, append it after the fully qualified name
   // with a '-' separator character.
   if ( ! qualifier.empty() )
   {
      full_name += std::string("-") + qualifier;
   }

   // Join the fully qualified plug-in name from its version number using the
   // separator string.
   full_name += std::string(sSeparator);

   // Turn the version number into a string where the version number values
   // are separatored by the '.' character.
   std::ostringstream version_stream;
   std::copy(version.begin(), version.end(),
             std::ostream_iterator<unsigned int>(version_stream, "."));
   full_name += version_stream.str();

   // The ostream iterator will have appended an extra '.' character when it
   // constructed the version string, and we need to chop it off.
   full_name.erase(full_name.size() - 1);

   return full_name;
}

std::string Info::buildNamespaceName(const std::string& nameSpace,
                                     const std::string& shortName)
{
   std::string ns_name(shortName);

   if ( ! nameSpace.empty() )
   {
      ns_name = nameSpace + std::string(".") + ns_name;
   }

   return ns_name;
}

}

}
