// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_TYPED_REGISTRY_ENTRY_H_
#define _INF_TYPED_REGISTRY_ENTRY_H_

#include <IOV/Config.h>

#include <sstream>

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/cast.hpp>

#include <vpr/DynLoad/Library.h>

#include <IOV/PluginCreator.h>
#include <IOV/RegistryEntry.h>
#include <IOV/Util/Exceptions.h>


namespace inf
{

/**
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
            boost::polymorphic_downcast<inf::PluginCreator<T>*>(
               getCreatorFunc(module, T::getCreatorFuncName())
            );
      }
      else
      {
         std::ostringstream msg_stream;
         msg_stream << "Module '" << module->getName() << "' is invalid!";
         throw inf::PluginException(msg_stream.str(), IOV_LOCATION);
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
   inf::PluginCreator<T>* getCreator() const
   {
      return mCreator;
   }

   virtual plugin_ptr_type doCreate()
   {
      return mCreator->createPlugin();
   }

private:
   inf::PluginCreator<T>* mCreator;
};

}


#endif /* _INF_TYPED_REGISTRY_ENTRY_H_ */
