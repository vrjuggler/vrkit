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

#include <utility>

#include <vrkit/plugin/Data.h>


namespace vrkit
{

namespace plugin
{

Data::InstanceStore Data::sInstanceStore;

Data::Data(const data::TypeDesc& desc, const std::string& name)
   : mName(name)
   , mInstanceID(0)
   , mTypeDesc(desc)
{
   typedef data::TypeDesc::const_iterator iter_type;
   for ( iter_type i = desc.begin(); i != desc.end(); ++i )
   {
      mData[(*i).getName()] = (*i).getInitValue();
   }
}

DataPtr Data::create(const data::TypeDesc& desc, const std::string& name)
{
   DataPtr ptr = DataPtr(new Data(desc, name));
   ptr->mInstanceID = sInstanceStore.add(ptr);
   return ptr;
}

const boost::any& Data::doGet(const std::string& key) const
{
   if ( mData.count(key) == 0 )
   {
      std::ostringstream msg_stream;
      msg_stream << "Invalid key '" << key << "' requested";
      throw vrkit::Exception(msg_stream.str(), VRKIT_LOCATION);
   }

   return (*mData.find(key)).second;
}

Data::InstanceStore::InstanceStore()
   // This starts at 1 so that pointer ID 0 can act as the universal
   // "null" pointer.
   : mCount(1)
{
   /* Do nothing. */ ;
}

Data::InstanceStore::ptr_type Data::InstanceStore::add(DataPtr i)
{
   const ptr_type ptr_value = mCount++;
   mInstances[ptr_value] = i;
   mNamedInstances.insert(std::make_pair(i->getName(), i));
   return ptr_value;
}

void Data::InstanceStore::remove(const ptr_type ptr)
{
   std::map<ptr_type, DataWeakPtr>::iterator i = mInstances.find(ptr);

   if ( i != mInstances.end() )
   {
      DataPtr obj = (*i).second.lock();
      mInstances.erase(i);

      // If the object identified by ptr is non-NULL, then it is also non-NULL
      // in the multimap mNamedInstances.
      if ( obj.get() != NULL )
      {
         // Get the range of objects with the same name as obj.
         typedef std::multimap<std::string, DataWeakPtr>::iterator iter_type;
         typedef std::pair<iter_type, iter_type> range_type;
         range_type range = mNamedInstances.equal_range(obj->getName());

         for ( iter_type o = range.first; o != range.second; ++o )
         {
            // This seems unnecessarily expensive. There must be a better way
            // to do by-name object lookups than using a multi-map.
            if ( obj == (*o).second.lock() )
            {
               mNamedInstances.erase(o);
               break;
            }
         }
      }
   }
}

const DataPtr Data::InstanceStore::get(const ptr_type ptr) const
{
   DataPtr instance;

   if ( mInstances.count(ptr) != 0 )
   {
      instance = (*mInstances.find(ptr)).second.lock();
   }

   return instance;
}

const DataPtr Data::InstanceStore::getByName(const vpr::GUID& typeID,
                                             const std::string& objName) const
{
   DataPtr instance;

   typedef std::multimap<std::string, DataWeakPtr>::const_iterator iter_type;
   typedef std::pair<iter_type, iter_type> range_type;
   range_type range = mNamedInstances.equal_range(objName);

   for ( iter_type o = range.first; o != range.second; ++o )
   {
      DataPtr temp_inst = (*o).second.lock();

      if ( temp_inst.get() != NULL && typeID == temp_inst->getTypeID() )
      {
         instance = temp_inst;
         break;
      }
   }

   return instance;
}

}

}
