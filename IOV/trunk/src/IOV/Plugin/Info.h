// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _IOV_PLUGIN_INFO_H_
#define _IOV_PLUGIN_INFO_H_

#include <IOV/Config.h>

#include <string>
#include <vector>


namespace inf
{

namespace plugin
{

/**
 * This type provides metadata about plug-in types that are loaded and used by
 * IOV applications. In this version, the metadata that can be attached to
 * a plug-in type is restricted to the following pre-defined kinds:
 *    - Namespace: An optional namespace used to avoid name conflicts.
 *    - Short name: A short (or "common") name for the plug-in type.
 *    - Version: A sequence of one or more unsigned integers.
 *    - Qualifier: An optional user-defined qualifier to distinguish between
 *      variants of the same plug-in.
 *    - Full name: The fually qualified name constructed from the above.
 *    - Dependencies: The plug-ins upon which the associated plug-in type
 *      depends.
 *
 * @since 0.36
 */
class IOV_CLASS_API Info
{
public:
   /**
    * Constructor.
    *
    * @pre No spaces appear in \p nameSpace, \p shortName, or \p qualifier.
    * @post The full name for the plug-in type is constructed from the given
    *       identifier attributes and stored in \c mFullName.
    *
    * @param nameSpace    The namespace for the plug-in type. This may be an
    *                     empty string if no namespace is necessary or
    *                     desired. The convention is to use Java-style naming
    *                     such as "com.infiscape".
    * @param shortName    The short or "common" name for the plug-in type.
    *                     This is a simple name that will normally be how
    *                     users refer to the plug-in type. For example, such a
    *                     name may be "GrabPlugin".
    * @param version      The version number for the plug-in type expressed as
    *                     a vector of unsigned integers. This will be
    *                     concatenated as necessary using an appropriate
    *                     character (usually '.'). The order of the vector
    *                     must be in decreasing order of significance. For
    *                     example, \p version[0] should be what is
    *                     traditionally thought of as the major version
    *                     number. The actual meaning of the version number is
    *                     left up to the plug-in implementor to decide.
    * @param qualifier    An optional qualifier to distinguish a plug-in type
    *                     variant. For example, this could be used to identify
    *                     that a build of the plug-in code has a debug-enabled
    *                     variant that can be chosen in addition to the
    *                     optimized variant. In that case, a logical choice
    *                     for the qualifier in the debug case would be
    *                     "debug".
    * @param dependencies A list of string identifiers for the dependencies of
    *                     the plug-in type associated with this informational
    *                     object. These strings may contain fully qualified
    *                     information (namespace, short name, qualifier, and
    *                     version [partial or full]) or the plug-in type name
    *                     with its namespace.
    *
    * @note If \p version is an empty vector, the version information for the
    *       associate plug-in type will be set to 0.0.0.
    *
    * @see buildFullName()
    */
   Info(
      const std::string& nameSpace, const std::string& shortName,
      const std::vector<unsigned int>& version,
      const std::string& qualifier = "",
      const std::vector<std::string>& dependencies = std::vector<std::string>()
   );

   /**
    * Returns the namespace for the plug-in type. This may be an empty string.
    */
   const std::string& getNamespace() const
   {
      return mNamespace;
   }

   /**
    * Returns the short (or "common") name for the plug-in type.
    */
   const std::string& getShortName() const
   {
      return mShortName;
   }

   const std::string& getNamespaceName() const
   {
      return mNamespaceName;
   }

   /**
    * Returns the version of the plug-in type encoded as a sequence of
    * unsigned integers. The returned vector is guaranteed to contain at least
    * one value.
    */
   const std::vector<unsigned int>& getVersion() const
   {
      return mVersion;
   }

   /**
    * Returns the user-defined qualifier for the plug-in type to distinguish
    * it from other variants of the same. This may be an empty string.
    */
   const std::string& getQualifier() const
   {
      return mQualifier;
   }

   /**
    * Returns the fully qualified name for the plug-in type associated with
    * this informational object.
    */
   const std::string& getFullName() const
   {
      return mFullName;
   }

   const std::vector<std::string>& getDependencies() const
   {
      return mDependencies;
   }

   bool dependsOn(const Info& otherInfo) const;

   /**
    * Constructs the fully qualified name for the plug-in type. Refer to the
    * documentation for the inf::plugin::Info constructor for the details of
    * each of the parameters for this function.
    *
    * @param nameSpace The namespace for the plug-in type.
    * @param shortName The short or "common" name for the plug-in type.
    * @param version   The version number for the plug-in type expressed as a
    *                  vector of unsigned integers.
    * @param qualifier An optional qualifier to distinguish a plug-in type
    *                  variant.
    *
    * @return The fully qualified name for a plug-in type is returned as a
    *         std::string object. The value of the string is, of course, based
    *         on the given inputs.
    */
   static std::string buildFullName(const std::string& nameSpace,
                                    const std::string& shortName,
                                    const std::vector<unsigned int>& version,
                                    const std::string& qualifier = "");

   /**
    * Constructs the "namespace name" for the plug-in type. This is the
    * concatenation of the namespace (if not empty) and the short naame. Refer
    * to the documentation for the inf::plugin::Info constructor for the
    * details of each of the parameters for this function.
    *
    * @param nameSpace The namespace for the plug-in type.
    * @param shortName The short or "common" name for the plug-in type.
    *
    * @return The namespace name for a plug-in type is returned as a
    *         std::string object. The value of the string is, of course, based
    *         on the given inputs.
    */
   static std::string buildNamespaceName(const std::string& nameSpace,
                                         const std::string& shortName);

   /**
    * Returns the string used to separate the name of the plug-in type (with
    * its optional namespace and qualifier) from its version.
    */
   static const std::string& getSeparator()
   {
      return sSeparator;
   }

private:
   static std::string sSeparator;

   /** @name Basic Plug-in Type Identifier Attributes */
   //@{
   std::string               mNamespace;        /**< Namespace (optional) */
   std::string               mShortName;        /**< Short name */
   std::vector<unsigned int> mVersion;          /**< Version */
   std::string               mQualifier;        /**< Qualifier (optional) */
   //@}

   /** Short name with namespace */
   std::string mNamespaceName;

   /**
    * The fully qualified name for the plug-in type. Ideally, this will
    * identify the associated plug-in type uniquely with respect to other
    * versions and/or variants of the same type.
    */
   std::string mFullName;

   /**
    * The dependencies of the associated plug-in type that must be loaded
    * and instantiated before our plug-in type can be instantiated.
    */
   std::vector<std::string> mDependencies;
};

}

}


#endif /* _IOV_PLUGIN_INFO_H_ */
