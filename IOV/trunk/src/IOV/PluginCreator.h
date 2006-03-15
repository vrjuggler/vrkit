// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_PLUGIN_CREATOR_H_
#define _INF_PLUGIN_CREATOR_H_

#include <IOV/Config.h>

#include <sstream>
#include <string>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <vpr/vpr.h>


namespace inf
{

template<typename PLUGIN_TYPE>
class PluginCreator
{
public:
   typedef boost::shared_ptr<PLUGIN_TYPE> plugin_ptr_t;
   typedef boost::function<plugin_ptr_t ()> creator_t;

   PluginCreator(creator_t creator, const std::string& pluginName,
                 const vpr::Uint32 pluginMajorVer = 1,
                 const vpr::Uint32 pluginMinorVer = 0,
                 const vpr::Uint32 pluginPatchVer = 0)
      : mCreator(creator)
      , mPluginName(pluginName)
      , mPluginMajorVer(pluginMajorVer)
      , mPluginMinorVer(pluginMinorVer)
      , mPluginPatchVer(pluginPatchVer)
   {
      std::ostringstream str_stream;
      str_stream << mPluginMajorVer << "." << mPluginMinorVer << "."
                 << mPluginPatchVer;
      mPluginVersionStr = str_stream.str();
   }

   ~PluginCreator()
   {
      /* Do nothing. */ ;
   }

   plugin_ptr_t createPlugin() const
   {
      return mCreator();
   }

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

private:
   creator_t mCreator;

   std::string mPluginName;

   vpr::Uint32 mPluginMajorVer;
   vpr::Uint32 mPluginMinorVer;
   vpr::Uint32 mPluginPatchVer;
   std::string mPluginVersionStr;
};

}


#endif
