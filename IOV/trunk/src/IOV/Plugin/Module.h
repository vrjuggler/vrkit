// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_PLUGIN_MODULE_H_
#define _INF_PLUGIN_MODULE_H_

#include <IOV/Config.h>

#include <string>
#include <boost/function.hpp>
#include <boost/type_traits/add_pointer.hpp>

#include <vpr/DynLoad/Library.h>


namespace inf
{

namespace plugin
{

/**
 * A wrapper around a vpr::LibraryPtr object that refers to a dynamically
 * loaded code unit containing a plug-in designed for use with IOV.
 *
 * @see inf::AbsstractPlugin
 * @see inf::PluginRepository
 *
 * @since 0.36
 */
class IOV_CLASS_API Module
{
public:
   Module(vpr::LibraryPtr module);

   /**
    * Returns the contained vpr::LibraryPtr object that knows about the
    * dynamically loaded code unit.
    */
   vpr::LibraryPtr getLibrary() const
   {
      return mModule;
   }

   /**
    * Determines whether the contained dynamically loaded code unit has the
    * given symbol.
    *
    * @param name The name of the symbol to be looked up in the dynamically
    *             located code unit contained within this object.
    *
    * @return true is returned if \p name is in the symbol table of the
    *         contained dynamically loaded code unit; false otherwise.
    */
   bool hasSymbol(const std::string& name) const;

   /**
    * Looks up the named symbol in the contained dynamically loaded code unit
    * and returns a function pointer to it. The template parameter
    * \c Signature must be suitable for use in creating an instantiation of
    * boost::function<T>.
    *
    * @param name The name of the symbol to be looked up in the dynamically
    *             located code unit contained within this object.
    *
    * @throw inf::PluginInterfaceException Thrown when \p name is not a symbol
    *        in the contained dynamically loaded code unit.
    *
    * @note If the given signature does not match the signature of the
    *       function symbol identified by \p name, the results are undefined.
    */
   template<typename Signature>
   boost::function<Signature> getFunction(const std::string& name) const
   {
      // For the casting operation to work, we have to pass the C-style
      // function pointer type as the template parameter for
      // reinterpret_cast<>. The type of Signature, however, is the C++-style
      // template paramter type for a function signature. For example, if we
      // have a function that returns an int and takes a const char* paramter,
      // we need the following:
      //
      //    Signature = int (const char*)
      //    CFuncType = int (*)(const char*)
      typedef typename boost::add_pointer<Signature>::type CFuncType;
      return reinterpret_cast<CFuncType>(getSymbol(name));
   }

private:
   /**
    * Retrieves a pointer to the named symboal in the contained dynamically
    * loaded code unit.
    *
    * @param name The name of the symbol to be looked up in the dynamically
    *             located code unit contained within this object.
    *
    * @return A non-NULL void pointer to the symbol table entry for \p is
    *         returned to the caller.
    *
    * @throw inf::PluginInterfaceException Thrown when \p name is not a symbol
    *        in the contained dynamically loaded code unit.
    */
   void* getSymbol(const std::string& name) const;

   vpr::LibraryPtr mModule;
};

}

}


#endif /* _INF_PLUGIN_MODULE_H_ */
