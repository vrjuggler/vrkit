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

#ifndef _VRKIT_PLUGIN_DATA_H_
#define _VRKIT_PLUGIN_DATA_H_

#include <vrkit/Config.h>

#include <sstream>
#include <string>
#include <map>
#include <typeinfo>
#include <boost/noncopyable.hpp>
#include <boost/any.hpp>
#include <boost/signal.hpp>

#include <vpr/Util/GUID.h>

#include <vrkit/Exception.h>
#include <vrkit/signal/Proxy.h>
#include <vrkit/plugin/data/ValueDesc.h>
#include <vrkit/plugin/data/TypeDesc.h>
#include <vrkit/plugin/DataPtr.h>


namespace vrkit
{

namespace plugin
{

/**
 * An instantiation of a specific vrkit plug-in data type defined at run time.
 * The exact structure of any given instantiation is determined by an XML
 * description. Every type is identified uniquely by a GUID and is composed of
 * one or more of the following types:
 *
 *  - bool: A boolean value, either true or false
 *  - uint8: An unsigned 8-bit value
 *  - int8: A signed 8-bit value
 *  - uint16: An unsigned 16-bit value
 *  - int16 A signed 16-bit value
 *  - uint32: An unsigned 32-bit value
 *  - int32: A signed 32-bit value
 *  - uint64: An unsigned 64-bit value
 *  - int64: A signed 64-bit value
 *  - float: A single-precision floating-point value
 *  - double: A double-precision floating-point value
 *  - string: A string of 8-bit characters
 *  - pointer: A pointer to another vrkit::plugin::Data object
 *
 * User code cannot create instances of vrkit::plugin::Data directly and
 * instead must ask the Data Factory (vrkit::plugin::DataFactory) to create
 * instances using the GUID for the desired type.
 *
 * When an instance is created, it must be given a name. Once given, the name
 * is fixed. It is used for looking up instances at run time. Thus, one plug-in
 * can create an instance of a dynamically defined data type, and another can
 * retrieve that instance by looking it up by name.
 *
 * @see vrkit::plugin::DataFactory
 *
 * @since 0.51.1
 */
class VRKIT_CLASS_API Data
   : private boost::noncopyable
{
private:
   /**
    * Creates an instance of a dynamically defined type using the given type
    * description.
    *
    * @post This instance is registered in the global instance store using a
    *       unique pointer value.
    */
   Data(const data::TypeDesc& desc, const std::string& name);

   static DataPtr create(const data::TypeDesc& desc, const std::string& name);

   friend class DataFactory;

public:
   /**
    * Removes this instance from the global instance store.
    */
   ~Data()
   {
      sInstanceStore.remove(mInstanceID);
      mInstanceID = 0;
   }

   /**
    * Returns the description for this dynamically defined type.
    */
   const data::TypeDesc& getDescription() const
   {
      return mTypeDesc;
   }

   /**
    * Returns the unique type identifier for the dynamically defined type of
    * this object.
    *
    * @see vrkit::plugin::data::TypeDesc
    */
   const vpr::GUID& getTypeID() const
   {
      return mTypeDesc.getID();
   }

   /**
    * Returns the name of this instance. The name is set at the time of
    * creation and cannot be changed during the lifetime of this object. The
    * primary purpose of naming instances is to allow them to be looked up at
    * run time.
    *
    * @see vrkit::plugin::DataFactory::findInstance()
    */
   const std::string& getName() const
   {
      return mName;
   }

   /**
    * Returns the named data member value as an instance of the C++ type
    * given as the template paramter.
    *
    * @param key The name of the data member whose value will be retrieved.
    *
    * @throw vrkit::Exception
    *           Thrown if \p key is not a valid data member for this
    *           dynamically defined type.
    * @throw boost::bad_any_cast
    *           Thrown if the C++ type identified in the template parameter
    *           does not match the C++ type of the stored data member
    *           identified by the given key.
    */
   template<typename T>
   T get(const std::string& key)
   {
      return boost::any_cast<T>(doGet(key));
   }

   /**
    * Returns the named data member value as an instance of the C++ type
    * given as the template paramter.
    *
    * @param key The name of the data member whose value will be retrieved.
    *
    * @throw vrkit::Exception
    *           Thrown if \p key is not a valid data member for this
    *           dynamically defined type.
    * @throw boost::bad_any_cast
    *           Thrown if the C++ type identified in the template parameter
    *           does not match the C++ type of the stored data member
    *           identified by the given key.
    */
   template<typename T>
   const T get(const std::string& key) const
   {
      return boost::any_cast<T>(doGet(key));
   }

   /**
    * Sets the value of the named data member to be the given value.
    *
    * @post If the data member named by \p key exists in this dynamically
    *       defined type exists and its type matches \p T, then its value
    *       matches \p value.
    *
    * @param key   The name of the data member to be modified.
    * @param value The value to assign to the named data member.
    *
    * @throw vrkit::Exception
    *           Thrown if \p key is not a valid data member for this
    *           dynamically defined type.
    *           Thrown if the C++ type identified given by the template
    *           parameter to this method does not match the C++ type of the
    *           data associated with the named key.
    */
   template<typename T>
   void set(const std::string& key, const T& value)
   {
      boost::any& v = doGet(key);

      // XXX: Using some sort of run-time is-convertible test here would be
      // a lot better than this approach. It would have to work something
      // like this:
      //
      //    if ( is_convertible(typed(t), v.type()) )
      //    {
      //       v = magic_cast<v.type()>(value)
      //    }
      //
      // Having this would avoid run-time errors that would normally not be
      // seen with C/C++.
      if ( v.type() == typeid(T) )
      {
         const boost::any old_value(v);
         v = value;
         mValueChanged(key, old_value, v);
      }
      else
      {
         std::ostringstream msg_stream;
         msg_stream << "Assignment failed: Stored type " << v.type().name()
                    << " does not match value type " << typeid(T).name();
         throw vrkit::Exception(msg_stream.str(), VRKIT_LOCATION);
      }
   }

   typedef boost::signal<
      void (const std::string&, const boost::any&, const boost::any&)
   > value_change_signal_t;

   /**
    * Provides access to the signal emitted when the value associated with a
    * data member in this dynamically defined type is modified.
    *
    * @see set<T>()
    */
   vrkit::signal::Proxy<value_change_signal_t> valueChanged()
   {
      return vrkit::signal::Proxy<value_change_signal_t>(mValueChanged);
   }

private:
   /**
    * This type acts as the instance storage for all vrkit::plugin::Data
    * objects that are created using the Data Factory. When such instances
    * are deleted, they are removed from the instance store.
    */
   class InstanceStore
   {
   public:
      InstanceStore();

      typedef data::TypeDesc::ptr_type ptr_type;

      /**
       * Registers the given vrkit::plugin::Data object and returns the
       * unique identifier for later indexing of that object in this instance
       * store.
       *
       * @post A weak pointer to the given object instance is stored in this
       *       instance store with a unique index.
       *
       * @param i An instance of vrkit::plugin::Data.
       */
      ptr_type add(DataPtr i);

      /**
       * Removes the vrkit::plugin::Data instance identified by the given
       * pointer index from this instance store.
       *
       * @post No object indexed by the value \p ptr is in \c mInstances.
       */
      void remove(const ptr_type ptr);

      /**
       * Retrieves the vrkit::plugin::Data object identified by the given
       * index value. If no such object exists or the object has already been
       * deleted but not removed from this instance store, a null shared
       * pointer is returned.
       */
      const DataPtr get(const ptr_type ptr) const;

      /**
       * Finds the named instance of the identified dynamically defined data
       * type.
       *
       * @param typeID  The unique type identifier for the object to be looked
       *                up.
       * @param objName The name of the object instance to be looked up.
       *
       * @return A non-null shared pointer to a vrkit::plugin::Data instance
       *         is returned if the search criteria find the desired object.
       *         Otherwise, a null shared pointer is returned.
       *
       * @since 0.51.2
       */
      const DataPtr getByName(const vpr::GUID& typeID,
                              const std::string& objName) const;

   private:
      ptr_type mCount;
      std::map<ptr_type, DataWeakPtr> mInstances;

      /**
       * The store of instances, indexed by name. This uses a weak pointer so
       * that the objects can be deleted externally. If this were not a weak
       * pointer, memory leaks would result because this object, effectively
       * a global, would always hold a reference to every instantiated
       * vrkit::plugin::Data object.
       */
      std::multimap<std::string, DataWeakPtr> mNamedInstances;
   };

   /**
    * Returns the named data member value as an instance of the C++ type
    * given as the template paramter.
    *
    * @param key The name of the data member whose value will be retrieved.
    *
    * @throw vrkit::Exception
    *           Thrown if \p key is not a valid data member for this
    *           dynamically defined type.
    * @throw boost::bad_any_cast
    *           Thrown if the C++ type identified in the template parameter
    *           does not match the C++ type of the stored data member
    *           identified by the given key.
    */
   boost::any& doGet(const std::string& key)
   {
      // Call the const version of this method.
      return
         const_cast<boost::any&>(static_cast<const Data*>(this)->doGet(key));
   }

   /**
    * Returns the named data member value as an instance of the C++ type
    * given as the template paramter.
    *
    * @param key The name of the data member whose value will be retrieved.
    *
    * @throw vrkit::Exception
    *           Thrown if \p key is not a valid data member for this
    *           dynamically defined type.
    * @throw boost::bad_any_cast
    *           Thrown if the C++ type identified in the template parameter
    *           does not match the C++ type of the stored data member
    *           identified by the given key.
    */
   const boost::any& doGet(const std::string& key) const;

   static InstanceStore sInstanceStore; /**< Global instance store */

   std::string mName;   /**< The (read-only) name of this instance. */

   /**
    * The value held by this member identifies this object uniquely within
    * the global instance store.
    */
   InstanceStore::ptr_type mInstanceID;

   /**
    * The type description for this dynamically defined data type.
    */
   data::TypeDesc mTypeDesc;

   /** @name Data Member Management */
   //@{
   typedef std::map<std::string, boost::any> data_map_t;

   /**
    * The collection of named data members for this dynamically defined data
    * type.
    */
   data_map_t mData;

   /**
    * Signal emitted when a value within this dynamically defined data type
    * changes.
    *
    * @see set<T>()
    */
   value_change_signal_t mValueChanged;
   //@}
};

/**
 * Specialization for vrkit::plugin::DataPtr to perform pointer lookups in the
 * global instance store.
 *
 * @param key The name of the data member whose value will be retrieved.
 *
 * @throw vrkit::Exception
 *           Thrown if \p key is not a valid data member for this dynamically
 *           defined type.
 */
template<>
inline const DataPtr Data::get<DataPtr>(const std::string& key) const
{
   return sInstanceStore.get(get<InstanceStore::ptr_type>(key));
}

/**
 * Specialization for vrkit::plugin::DataPtr to perform pointer lookups in the
 * global instance store.
 *
 * @param key The name of the data member whose value will be retrieved.
 *
 * @throw vrkit::Exception
 *           Thrown if \p key is not a valid data member for this dynamically
 *           defined type.
 */
template<>
inline DataPtr Data::get<DataPtr>(const std::string& key)
{
   // Call the const version of this method.
   return static_cast<const Data*>(this)->get<DataPtr>(key);
}

/**
 * Specialization for vrkit::plugin::DataPtr to perform pointer lookups in the
 * global instance store.
 *
 * @post If the data member named by \p key exists in this dynamically
 *       defined type exists and the type of \p ptr is accepted pointee type,
 *       then the pointer value for \p ptr is stored in the named data member.
 *
 * @param key The name of the data member to be modified.
 * @param ptr The pointer to assign to the named data member.
 *
 * @throw vrkit::Exception
 *           Thrown if the type of \p ptr is not accepted as a pointer value
 *           for the named data member. This also covers the case of an
 *           attempt to assign a pointer to a non-pointer data member.
 *           Thrown if \p key is not a valid data member for this
 *           dynamically defined type.
 */
template<>
inline void Data::set(const std::string& key, const DataPtr& ptr)
{
   // If the value desciption for key is not for a pointer, its pointee type
   // will be a null GUID, which is not a valid pointee type. Thus, this
   // condition will evaluate to false.
   if ( mTypeDesc.getValueDesc(key).acceptsPointee(ptr->getTypeID()) )
   {
      set(key, ptr->mInstanceID);
   }
   else
   {
      std::ostringstream msg_stream;
      msg_stream << "Pointee type " << ptr->getDescription().getID()
                 << "\nis not allowed for member '" << key << "'";
      throw vrkit::Exception(msg_stream.str(), VRKIT_LOCATION);
   }
}

}

}


#endif /* _VRKIT_PLUGIN_DATA_H_ */
