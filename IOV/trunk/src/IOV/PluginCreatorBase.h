// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_PLUGIN_CREATOR_BASE_H_
#define _INF_PLUGIN_CREATOR_BASE_H_

#include <IOV/Config.h>

#include <boost/noncopyable.hpp>


namespace inf
{

/**
 * Base class for plug-in creators. It does not actually create anything but
 * instead serves to allow derived types to be used in polymorphic contexts.
 * This class cannot be instantiated directly.
 *
 * @since 0.16.0
 */
class IOV_CLASS_API PluginCreatorBase : private boost::noncopyable
{
protected:
   PluginCreatorBase();

public:
   virtual ~PluginCreatorBase();
};

}


#endif
