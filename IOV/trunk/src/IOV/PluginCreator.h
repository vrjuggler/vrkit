// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_PLUGIN_CREATOR_H_
#define _INF_PLUGIN_CREATOR_H_

#include <IOV/Config.h>

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include <IOV/PluginCreatorBase.h>


namespace inf
{

template<typename PLUGIN_TYPE>
class PluginCreator : public inf::PluginCreatorBase
{
public:
   typedef boost::shared_ptr<PLUGIN_TYPE> plugin_ptr_t;
   typedef boost::function<plugin_ptr_t ()> creator_t;

   PluginCreator(creator_t creator)
      : mCreator(creator)
   {
      /* Do nothing. */ ;
   }

   virtual ~PluginCreator()
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
