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

#ifndef _VRKIT_PLUGIN_TYPED_REGISTRY_ENTRY_H_
#define _VRKIT_PLUGIN_TYPED_REGISTRY_ENTRY_H_

#include <vrkit/Config.h>

#include <sstream>

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/cast.hpp>

#include <vpr/DynLoad/Library.h>

#include <vrkit/plugin/Creator.h>
#include <vrkit/plugin/RegistryEntry.h>
#include <vrkit/exceptions/PluginException.h>


namespace vrkit
{

namespace plugin
{

/**
 * @note This class was moved into the vrkit::plugin namespace in version 0.47.
 *
 * @since 0.36
 */
template<typename T>
class TypedRegistryEntry
   : public RegistryEntry
{
public:
   typedef boost::shared_ptr<TypedRegistryEntry> ptr_type;

   /** @name Types for Plug-ins */
   //@{
   typedef boost::function<bool (vpr::LibraryPtr)> validator_func_type;
   typedef typename T::ptr_type plugin_ptr_type;
   //@}

protected:
   TypedRegistryEntry(vpr::LibraryPtr module, validator_func_type validator)
      : RegistryEntry(module)
      , mCreator(NULL)
   {
      if ( validator(module) )
      {
         // RegistryEntry::getCreatorFunc() will throw an exception if the
         // creator function could not be found.
         mCreator =
            boost::polymorphic_downcast<Creator<T>*>(
               getCreatorFunc(module, T::getCreatorFuncName())
            );
      }
      else
      {
         std::ostringstream msg_stream;
         msg_stream << "Module '" << module->getName() << "' is invalid!";
         throw PluginException(msg_stream.str(), VRKIT_LOCATION);
      }
   }

public:
   static RegistryEntryPtr create(vpr::LibraryPtr module,
                                  validator_func_type validator)
   {
      return RegistryEntryPtr(new TypedRegistryEntry(module, validator));
   }

   virtual ~TypedRegistryEntry()
   {
      /* Do nothing. */ ;
   }

   virtual AbstractPluginPtr create()
   {
      return doCreate();
   }

protected:
   Creator<T>* getCreator() const
   {
      return mCreator;
   }

   virtual plugin_ptr_type doCreate()
   {
      return mCreator->createPlugin();
   }

private:
   Creator<T>* mCreator;
};

}

}


#endif /* _VRKIT_PLUGIN_TYPED_REGISTRY_ENTRY_H_ */
