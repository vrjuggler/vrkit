// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_ABSTRACT_PLUGIN_H_
#define _INF_ABSTRACT_PLUGIN_H_

#include <IOV/Config.h>

#include <boost/noncopyable.hpp>

#include <vpr/DynLoad/Library.h>

#include <IOV/Plugin/Info.h>
#include <IOV/AbstractPluginPtr.h>


namespace inf
{

namespace plugin
{

class Module;

}

/**
 * @since 0.36
 */
class IOV_CLASS_API AbstractPlugin : private boost::noncopyable
{
protected:
   AbstractPlugin(const inf::plugin::Info& info);

public:
   virtual ~AbstractPlugin();

   static std::string getInfoFuncName()
   {
      return "getPluginInfo";
   }

   const inf::plugin::Info& getInfo() const
   {
      return mInfo;
   }

protected:
   /**
    * Validates that the given dynamic code entity has the query interface
    * needed for all IOV plug-ins.
    *
    * @pre \p pluginLib must have been loaded successfully.
    *
    * @param pluginLib The code entity (shared library, DLL, executable) to
    *                  validate.
    *
    * @throw inf::PluginInterfaceException is thrown if \p pluginLib is not a
    *        valid dynamic code entity for an IOV plug-in.
    */
   static void basicValidation(const inf::plugin::Module& module);

private:
   const inf::plugin::Info mInfo;
};

}


#endif /* _INF_ABSTRACT_PLUGIN_H_ */
