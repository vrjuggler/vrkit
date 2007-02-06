// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_PLUGIN_CREATOR_H_
#define _INF_PLUGIN_CREATOR_H_

#include <IOV/Config.h>

#include <string>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <vpr/vpr.h>

#include <IOV/PluginCreatorBase.h>


namespace inf
{

template<typename PLUGIN_TYPE>
class PluginCreator : public inf::PluginCreatorBase
{
public:
   typedef boost::shared_ptr<PLUGIN_TYPE> plugin_ptr_t;
   typedef boost::function<plugin_ptr_t ()> creator_t;

   PluginCreator(creator_t creator, const std::string& pluginName,
                 const vpr::Uint32 pluginMajorVer = 1,
                 const vpr::Uint32 pluginMinorVer = 0,
                 const vpr::Uint32 pluginPatchVer = 0)
      : PluginCreatorBase(pluginName, pluginMajorVer, pluginMinorVer,
                          pluginPatchVer)
      , mCreator(creator)
   {
      /* Do nothing. */ ;
   }

   ~PluginCreator()
   {
      /* Do nothing. */ ;
   }

   plugin_ptr_t createPlugin() const
   {
      return mCreator();
   }

private:
   creator_t mCreator;
};

}


#endif
