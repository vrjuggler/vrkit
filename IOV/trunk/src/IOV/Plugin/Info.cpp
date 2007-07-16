// Copyright (C) Infiscape Corporation 2005-2007

#include <iostream>
#include <sstream>
#include <iterator>
#include <algorithm>

#include <IOV/Plugin/Info.h>


namespace inf
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
