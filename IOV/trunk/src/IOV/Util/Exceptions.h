// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_IOV_EXCEPTIONS_H_
#define _INF_IOV_EXCEPTIONS_H_

#include <IOV/Config.h>

#include <stdexcept>
#include <string>

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
   Exception(const std::string& desc, const std::string& location) throw ();

   virtual ~Exception() throw ();

   virtual const char* what() const throw ();

   virtual std::string getExceptionName() const;

   std::string getDescription() const;
   void setDescription(const std::string& desc);

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
class IOV_CLASS_API PluginLoadException : public PluginException
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
class IOV_CLASS_API NoSuchPluginException : public PluginException
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

class IOV_CLASS_API PluginInterfaceException : public PluginException
{
public:
   PluginInterfaceException(const std::string& msg,
                            const std::string& location) throw ();

   virtual ~PluginInterfaceException() throw ();

   std::string getExceptionName()
   {
      return "inf::PluginInterfaceException";
   }
};

/**
 * @since 0.36
 */
class IOV_CLASS_API PluginTypeNameException : public PluginException
{
public:
   PluginTypeNameException(const std::string& msg,
                           const std::string& location) throw ();

   virtual ~PluginTypeNameException() throw ();

   std::string getExceptionName()
   {
      return "inf::PluginTypeNameException";
   }
};

/**
 * @since 0.36
 */
class IOV_CLASS_API PluginDependencyException : public PluginException
{
public:
   PluginDependencyException(const std::string& msg,
                             const std::string& location) throw ();

   virtual ~PluginDependencyException() throw ();

   std::string getExceptionName()
   {
      return "inf::PluginDependencyException";
   }
};

/**
 * Base recording exception type. All exception types associated with video
 * capture should derive from this class.
 *
 * @see inf::VideoCamera
 * @see inf::VideoEncoder
 * @see inf::Encoder
 *
 * @since 0.45.1
 */
class IOV_CLASS_API RecordingException : public Exception
{
public:
   RecordingException(const std::string& msg, const std::string& location)
      throw ();

   virtual ~RecordingException() throw ();

   std::string getExceptionName()
   {
      return "inf::RecordingException";
   }
};

/**
 * Exception thrown if an attempt is made to record OpenGL frames with an
 * invalid encoder configuration.
 *
 * @see inf::VideoCamera
 * @see inf::VideoEncoder
 *
 * @since 0.45.1
 */
class IOV_CLASS_API InvalidRecordingConfigException
   : public RecordingException
{
public:
   InvalidRecordingConfigException(const std::string& msg,
                                   const std::string& location) throw ();

   virtual ~InvalidRecordingConfigException() throw ();

   std::string getExceptionName()
   {
      return "inf::InvalidRecordingConfigException";
   }
};

/**
 * Exception thrown if an attempt is made to record OpenGL frames with bad
 * data.
 *
 * @see inf::VideoCamera
 * @see inf::VideoEncoder
 *
 * @since 0.45.1
 */
class IOV_CLASS_API BadRecordingDataException : public RecordingException
{
public:
   BadRecordingDataException(const std::string& msg,
                             const std::string& location) throw ();

   virtual ~BadRecordingDataException() throw ();

   std::string getExceptionName()
   {
      return "inf::BadRecordingDataException";
   }
};

}


#endif
