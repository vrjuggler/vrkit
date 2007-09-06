// vrkit is (C) Copyright 2005-2007
//    by Allen Bierbaum, Aron Bierbuam, Patrick Hartling, and Daniel Shipton
//
// This file is part of vrkit.
//
// vrkit is free software; you can redistribute it and/or modify it under the
// terms of the GNU Lesser General Public License as published by the Free
// Software Foundation; either version 2 of the License, or (at your option)
// any later version.
//
// vrkit is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
// more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef _VRKIT_PLUGIN_MODULE_H_
#define _VRKIT_PLUGIN_MODULE_H_

#include <vrkit/Config.h>

#include <string>
#include <boost/function.hpp>
#include <boost/type_traits/add_pointer.hpp>

#include <vpr/DynLoad/Library.h>


namespace vrkit
{

namespace plugin
{

/**
 * A wrapper around a vpr::LibraryPtr object that refers to a dynamically
 * loaded code unit containing a plug-in designed for use with vrkit.
 *
 * @see vrkit::AbsstractPlugin
 * @see vrkit::PluginRepository
 *
 * @since 0.36
 */
class VRKIT_CLASS_API Module
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
    * @throw vrkit::PluginInterfaceException
    *           Thrown when \p name is not a symbol in the contained
    *           dynamically loaded code unit.
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

      // GCC versions older than 4.0 do not handle the reinterpret_cast usage
      // below correctly.
#if defined(__GNUC__) && __GNUC_MAJOR__ < 4
      return (CFuncType) getSymbol(name);
#else
      return reinterpret_cast<CFuncType>(getSymbol(name));
#endif
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
    * @throw vrkit::PluginInterfaceException
    *           Thrown when \p name is not a symbol in the contained
    *           dynamically loaded code unit.
    */
   void* getSymbol(const std::string& name) const;

   vpr::LibraryPtr mModule;
};

}

}


#endif /* _VRKIT_PLUGIN_MODULE_H_ */
