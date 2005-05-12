#ifndef _INF_IOV_EXCEPTIONS_H_
#define _INF_IOV_EXCEPTIONS_H_

#include <stdexcept>


namespace inf
{

class PluginLoadException : public std::runtime_error
{
public:
   PluginLoadException(const std::string& msg) throw()
      : std::runtime_error(msg)
   {
   }

   virtual ~PluginLoadException() throw()
   {
   }
};

class NoSuchPluginException : public std::runtime_error
{
public:
   NoSuchPluginException(const std::string& msg) throw()
      : std::runtime_error(msg)
   {
   }

   virtual ~NoSuchPluginException() throw()
   {
   }
};

class PluginInterfaceException : public std::runtime_error
{
public:
   PluginInterfaceException(const std::string& msg) throw()
      : std::runtime_error(msg)
   {
   }

   virtual ~PluginInterfaceException() throw()
   {
   }
};

}


#endif
