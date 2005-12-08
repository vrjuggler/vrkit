// Copyright (C) Infiscape Corporation 2005

#ifndef _INF_IOV_EXCEPTIONS_H_
#define _INF_IOV_EXCEPTIONS_H_

#include <IOV/Config.h>

#include <stdexcept>

//#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/stringize.hpp>

#define IOV_LOCATION std::string(__FILE__) + std::string(":") + \
           std::string(BOOST_PP_STRINGIZE(__LINE__))

// Example:
//  throw inf::Exception("ObjectProxy not ready in Node::objProxy_checked", IOV_LOCATION);

// Exception areas
// - I/O loading/saving issues
// - Property access errors
// - Invalid data type errors


namespace inf
{

/** Base exception for all IOV exceptions.
 */
class IOV_CLASS_API Exception : public std::runtime_error
{
public:
   Exception(std::string desc, std::string location) throw ();

   virtual ~Exception() throw ();

   virtual const char* what() const throw ();

   virtual std::string getExceptionName() const;

   std::string getDescription() const;
   void setDescription(std::string desc);

   /** Slightly longer description */
   virtual std::string getExtendedDescription() const;

   /** Description with everything we know */
   virtual std::string getFullDescription() const;

protected:
   std::string mDescription;
   std::string mLocation;
   std::string mStackTrace;

   mutable std::string m_full_desc;    /**< Temporary string to return as char* where needed */
};

/** Exceptions dealing with plugin handling. */
class IOV_CLASS_API PluginException : public Exception
{
public:
   PluginException(const std::string& msg, const std::string& location = "")
      throw ();

   virtual ~PluginException() throw ();

   std::string getExceptionName()
   {
      return "inf::PluginException";
   }
};

/** Exception when loading a plugin. */
class PluginLoadException : public PluginException
{
public:
   PluginLoadException(const std::string& msg,
                       const std::string& location = "") throw ();

   virtual ~PluginLoadException() throw ();

   std::string getExceptionName()
   {
      return "inf::PluginLoadException";
   }
};

/** Exception when we can't find a plugin. */
class NoSuchPluginException : public PluginException
{
public:
   NoSuchPluginException(const std::string& msg,
                         const std::string& location = "") throw ();

   virtual ~NoSuchPluginException() throw ();

   std::string getExceptionName()
   {
      return "inf::NoSuchPluginException";
   }
};

class PluginInterfaceException : public PluginException
{
public:
   PluginInterfaceException(const std::string& msg,
                            const std::string& location) throw ();

   virtual ~PluginInterfaceException() throw ();

   std::string getExceptionName()
   {
      return "infi::PluginInterfaceException";
   }
};

}


#endif
