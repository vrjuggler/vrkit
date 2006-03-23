// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_PLUGIN_CREATOR_BASE_H_
#define _INF_PLUGIN_CREATOR_BASE_H_

#include <IOV/Config.h>

#include <string>
#include <boost/noncopyable.hpp>
#include <vpr/vpr.h>


namespace inf
{

/**
 * Base class for plug-in creators. It does not actually create anything but
 * instead serves to allow derived types to be used in polymorphic contexts.
 * This class cannot be instantiated directly.
 *
 * @since 0.16.0
 */
class IOV_CLASS_API PluginCreatorBase : boost::noncopyable
{
public:
   virtual ~PluginCreatorBase();

   const std::string& getPluginName() const
   {
      return mPluginName;
   }

   vpr::Uint32 getPluginMajorVersion() const
   {
      return mPluginMajorVer;
   }

   vpr::Uint32 getPluginMinorVersion() const
   {
      return mPluginMajorVer;
   }

   vpr::Uint32 getPluginPatchVersion() const
   {
      return mPluginPatchVer;
   }

   const std::string& getPluginVersion() const
   {
      return mPluginVersionStr;
   }

protected:
   PluginCreatorBase(const std::string& pluginName,
                     const vpr::Uint32 pluginMajorVer = 1,
                     const vpr::Uint32 pluginMinorVer = 0,
                     const vpr::Uint32 pluginPatchVer = 0);

   std::string mPluginName;

   vpr::Uint32 mPluginMajorVer;
   vpr::Uint32 mPluginMinorVer;
   vpr::Uint32 mPluginPatchVer;
   std::string mPluginVersionStr;
};

}


#endif
