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

/** \class TypedRegistryEntry TypedRegistryEntry.h vrkit/plugin/TypedRegistryEntry.h
 *
 * A plug-in registry entry type for vrkit::plugin::Registry for subclasses of
 * vrkit::AbstractPlugin. Specifically, the template parameter for
 * instantiations of this type must be a subclass (direct or indirect) of
 * vrkit::AbstractPlugin. The type of plug-in instance that is created by
 * the create() method is not strictly important because that type may not be
 * known at the time that vrkit and other users of this type are compiled.
 *
 * The instances created by this registry entry type have a simple
 * create-and-use procedure. Further initialization steps are not performed.
 * To perform post-creation initialization, use
 * vrkit::plugin::TypedInitRegistryEntry instead.
 *
 * @see vrkit::plugin::Creator
 * @see vrkit::plugin::TypedInitRegistryEntry.
 *
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
   /**
    * Constructor for the case when the creator function must be queried at
    * run time from the given plug-in module object.
    *
    * @param module    The dynamically loaded library from which the creator
    *                  will be retrieved.
    * @param validator A validator used to ensure that \p module provides
    *                  what is necessary to create instances of the plug-in.
    */
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

   /**
    * Constuctor for the case of a creator that is known statically at
    * compile time.
    *
    * @param creator The type-specific creator of the plug-in objects. This
    *                must refer to an object and a type \p T known at the time
    *                that this code is compiled.
    *
    * @since 0.51.0
    */
   TypedRegistryEntry(Creator<T>* creator)
      : RegistryEntry(vpr::LibraryPtr())
      , mCreator(creator)
   {
      /* Do nothing. */ ;
   }

public:
   /**
    * Creates a registry entry for vrrkit::plugin::Registry where the creator
    * function must be looked up at run time from a dynamically loaded
    * library.
    *
    * @param module    The dynamically loaded library from which the creator
    *                  will be retrieved.
    * @param validator A validator used to ensure that \p module provides
    *                  what is necessary to create instances of the plug-in.
    */
   static RegistryEntryPtr create(vpr::LibraryPtr module,
                                  validator_func_type validator)
   {
      return RegistryEntryPtr(new TypedRegistryEntry(module, validator));
   }

   /**
    * Creates a registry entry for vrkit::plugin::Registry where the creator
    * function is compiled into the code statically rather than being loaded
    * dynamically from a plug-in module. This is not appropriate for cases
    * when the creator function must be looked up at run time from a
    * dynamically loaded library. The object returned by
    * vrkit::plugin::RegistryEntry::getModule() will be a null shared pointer.
    *
    * @param creator The type-specific creator of the plug-in objects. This
    *                must refer to an object and a type \p T known at the time
    *                that this code is compiled.
    *
    * @since 0.51.0
    */
   static RegistryEntryPtr create(Creator<T>* creator)
   {
      return RegistryEntryPtr(new TypedRegistryEntry(creator));
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

   /**
    * Performs the work of creating the typed plug-in instance. This method
    * uses the Template Method Pattern so that subclasses can customize the
    * instantiation behavior.
    */
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
