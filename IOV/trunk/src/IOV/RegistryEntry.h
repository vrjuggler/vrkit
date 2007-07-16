// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_REGISTRY_ENTRY_H_
#define _INF_REGISTRY_ENTRY_H_

#include <IOV/Config.h>

#include <string>
#include <boost/noncopyable.hpp>

#include <vpr/DynLoad/Library.h>

#include <IOV/AbstractPluginPtr.h>
#include <IOV/Plugin/Info.h>
#include <IOV/RegistryEntryPtr.h>


namespace inf
{

class PluginCreatorBase;

/**
 * @since 0.36
 */
class IOV_CLASS_API RegistryEntry : private boost::noncopyable
{
protected:
   RegistryEntry(vpr::LibraryPtr module);

public:
   virtual ~RegistryEntry();

   const inf::plugin::Info& getInfo() const
   {
      return mModuleInfo;
   }

   vpr::LibraryPtr getModule() const
   {
      return mModule;
   }

   virtual inf::AbstractPluginPtr create() = 0;

protected:
   inf::PluginCreatorBase* getCreatorFunc(
      vpr::LibraryPtr module,
      const std::string& getCreatorFuncName
   ) const;

private:
   inf::plugin::Info getModuleInfo(vpr::LibraryPtr module);

   vpr::LibraryPtr   mModule;
   inf::plugin::Info mModuleInfo;
};

}


#endif /* _INF_REGISTRY_ENTRY_H_ */
