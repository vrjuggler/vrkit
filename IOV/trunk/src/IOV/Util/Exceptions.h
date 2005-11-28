#ifndef _VRS_IOV_EXCEPTIONS_H_
#define _VRS_IOV_EXCEPTIONS_H_

//#include <IOV/Config.h>

#include <stdexcept>

//#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/stringize.hpp>

#define VRS_LOCATION std::string(__FILE__) + std::string(":") + \
           std::string(BOOST_PP_STRINGIZE(__LINE__))

// Example:
//  throw vrs::Exception("ObjectProxy not ready in Node::objProxy_checked", VRS_LOCATION);

// Exception areas
// - I/O loading/saving issues
// - Property access errors
// - Invalid data type errors


namespace vrs
{

/** Base exception for all IOV exceptions.
 */
//class IOV_CLASS_API Exception : public std::runtime_error
class Exception : public std::runtime_error
{
public:
   Exception(std::string desc, std::string location) throw();
   virtual ~Exception() throw();

   virtual const char* what() const throw();

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
//class IOV_CLASS_API PluginException : public Exception
class PluginException : public Exception
{
public:
   PluginException(const std::string& msg, const std::string& location = "")
      throw();

   virtual ~PluginException() throw();

   std::string getExceptionName()
   { return "vrs::PluginException"; }
};

/** Exception when loading a plugin. */
class PluginLoadException : public PluginException
{
public:
   PluginLoadException(const std::string& msg,
                       const std::string& location = "") throw();

   virtual ~PluginLoadException() throw();

   std::string getExceptionName()
   { return "vrs::PluginLoadException"; }
};

/** Exception when we can't find a plugin. */
class NoSuchPluginException : public PluginException
{
public:
   NoSuchPluginException(const std::string& msg,
                         const std::string& location = "") throw();

   virtual ~NoSuchPluginException() throw();

   std::string getExceptionName()
   { return "vrs::NoSuchPluginException"; }
};

class PluginInterfaceException : public PluginException
{
public:
   PluginInterfaceException(const std::string& msg,
                            const std::string& location) throw();

   virtual ~PluginInterfaceException() throw();

   std::string getExceptionName()
   { return "vrs::PluginInterfaceException";  }
};

}


#endif
