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

#ifndef _VRKIT_PLUGIN_DATA_TYPE_REPOSITORY_H_
#define _VRKIT_PLUGIN_DATA_TYPE_REPOSITORY_H_

#include <vrkit/Config.h>

#include <string>
#include <map>
#include <utility>

#include <cppdom/cppdom.h>
#include <vpr/Util/GUID.h>

#include <vrkit/SceneData.h>
#include <vrkit/plugin/data/TypeDesc.h>
#include <vrkit/plugin/DataPtr.h>
#include <vrkit/plugin/DataFactoryPtr.h>


namespace vrkit
{

namespace plugin
{

/**
 * A factory for dynamically defined data types for use by vrkit plug-ins.
 * This factory is the only way to create instances of vrkit::plugin::Data.
 * This type is a subclass of vrkit::SceneData, meaning that it is supposed
 * to be instantiated/accessed through the vrkit::Scene object.
 *
 * @see vrkit::Viewer
 * @see vrkit::Scene
 *
 * @since 0.51.1
 */
class VRKIT_CLASS_API DataFactory
   : public vrkit::SceneData
{
private:
   /**
    * Scans the path identified by the environment variable
    * VRKIT_DATA_TYPE_PATH for \c .vdata files. These are XML files that
    * describe data types.
    */
   DataFactory();

public:
   /**
    * The unique type identifier for this scene data type.
    */
   static const vpr::GUID type_guid;

   static DataFactoryPtr create()
   {
      return DataFactoryPtr(new DataFactory());
   }

   /** @name Type Registration */
   //@{
   /**
    * Adds the described type to this data factory.
    */
   void registerType(cppdom::NodePtr declRoot)
   {
      data::TypeDesc desc(declRoot);

      // Basically equivalent to mTypes[desc.getID()] = desc
      // Using this form allows TypeDesc to be used without a default
      // constructor.
      mTypes.insert(std::make_pair(desc.getID(), desc));
   }

   /**
    * Adds the described type to this data factory. The given string must be
    * valid XML that describes a vrkit plug-in data type.
    */
   void registerType(const std::string& typeDecl);
   //@}

   /** @name Factory Interface */
   //@{
   /**
    * Creates an instance of the dynamically defined data type identified by
    * the given unique type identifier.
    *
    * @param guidStr A GUID (in string form) for a known dynamically defined
    *                data type.
    * @param objName The name for the created object. Once set, this cannot be
    *                changed. It is used for looking up instances at run time.
    *
    * @throw vrkit::Exception
    *           Thrown if the identified type is not known to this data
    *           factory.
    */
   DataPtr createInstance(const std::string& guidStr,
                          const std::string& objName)
   {
      return createInstance(vpr::GUID(guidStr), objName);
   }

   /**
    * Creates an instance of the dynamically defined data type identified by
    * the given unique type identifier.
    *
    * @param typeID  A GUID for a known dynamically defined data type.
    * @param objName The name for the created object. Once set, this cannot be
    *                changed. It is used for looking up instances at run time.
    *
    * @throw vrkit::Exception
    *           Thrown if the identified type is not known to this data
    *           factory.
    */
   DataPtr createInstance(const vpr::GUID& typeID,
                          const std::string& objName);

   /**
    * Finds the named instance of the identified dynamically defined data
    * type. It is important to understand that this method is \em not a
    * creation function. Instead, it looks for an extant object instance.
    *
    * @param typeID  The unique type identifier for the object to be looked
    *                up.
    * @param objName The name of the object instance to be looked up. This is
    *                a user-defined string. To get a statistically unique
    *                name, use a GUID. In any event, object names must be
    *                published so that consumer plug-ins have a way to find
    *                the objects that they need from the producer plug-ins.
    *
    * @return A non-null shared pointer to a vrkit::plugin::Data instance is
    *         returned if the search criteria find the desired object.
    *         Otherwise, a null shared pointer is returned.
    *
    * @since 0.51.2
    */
   DataPtr findInstance(const vpr::GUID& typeID, const std::string& objName)
      const;
   //}

private:
   /**
    * The collection of all registered dynamic data types. These are indexed
    * by the unique type identifier. Therefore, it is critical that no two
    * types use the same identifier.
    */
   std::map<vpr::GUID, data::TypeDesc> mTypes;
};

}

}


#endif /* _VRKIT_PLUGIN_DATA_TYPE_REPOSITORY_H_ */
