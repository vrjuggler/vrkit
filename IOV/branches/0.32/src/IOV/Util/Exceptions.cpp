// Copyright (C) Infiscape Corporation 2005-2007

#include <iostream>
#include <vpr/System.h>
#include <IOV/Util/Exceptions.h>


namespace inf
{

Exception::Exception(const std::string& desc, const std::string& location)
   throw ()
   : std::runtime_error(desc)
   , mDescription(desc)
   , mLocation(location)
{
   mStackTrace = vpr::System::getCallStack();
}

Exception::~Exception() throw ()
{
   ;
}

const char* Exception::what() const throw()
{
   m_full_desc = getFullDescription();
   return m_full_desc.c_str();
}

std::string Exception::getExceptionName() const
{
   return std::string("inf::Exception");
}

std::string Exception::getDescription() const
{
   return mDescription;
}

void Exception::setDescription(const std::string& desc)
{
   mDescription = desc;
}

std::string Exception::getExtendedDescription() const
{
   return this->getExceptionName() + std::string(": ") + getDescription();
}

std::string Exception::getFullDescription() const
{
   return getExtendedDescription() + std::string("  ") + mLocation +
             std::string("\n") + mStackTrace;
}

PluginException::PluginException(const std::string& msg,
                                 const std::string& location) throw ()
   : Exception(msg, location)
{
   /* Do nothing. */ ;
}

PluginException::~PluginException() throw ()
{
   /* Do nothign. */ ;
}

PluginLoadException::PluginLoadException(const std::string& msg,
                                         const std::string& location) throw ()
   : PluginException(msg, location)
{
   /* Do nothing. */ ;
}

PluginLoadException::~PluginLoadException() throw ()
{
   /* Do nothing. */ ;
}

NoSuchPluginException::NoSuchPluginException(const std::string& msg,
                                             const std::string& location)
   throw ()
   : PluginException(msg, location)
{
   /* Do nothing. */ ;
}

NoSuchPluginException::~NoSuchPluginException() throw ()
{
   /* Do nothing. */ ;
}

PluginInterfaceException::PluginInterfaceException(const std::string& msg,
                                                   const std::string& location)
   throw ()
   : PluginException(msg, location)
{
   /* Do nothing. */ ;
}

PluginInterfaceException::~PluginInterfaceException() throw ()
{
   /* Do nothing. */ ;
}

}
