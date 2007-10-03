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

#ifndef _VRKIT_ABSTRACT_PLUGIN_H_
#define _VRKIT_ABSTRACT_PLUGIN_H_

#include <vrkit/Config.h>

#include <boost/noncopyable.hpp>

#include <vpr/DynLoad/Library.h>

#include <vrkit/plugin/Info.h>
#include <vrkit/AbstractPluginPtr.h>


namespace vrkit
{

namespace plugin
{

class Module;

}

/** \class AbstractPlugin AbstractPlugin.h vrkit/AbstractPlugin.h
 *
 * This is the base class for all plug-in types handled by vrkit components.
 *
 * @see vrkit::plugin::Registry
 * @see vrkit::plugin::RegistryEntry
 * @see vrkit::plugin::Creator
 *
 * @since 0.36
 */
class VRKIT_CLASS_API AbstractPlugin : private boost::noncopyable
{
protected:
   AbstractPlugin(const plugin::Info& info);

public:
   virtual ~AbstractPlugin();

   static std::string getInfoFuncName()
   {
      return "getPluginInfo";
   }

   const plugin::Info& getInfo() const
   {
      return mInfo;
   }

protected:
   /**
    * Validates that the given dynamic code entity has the query interface
    * needed for all vrkit plug-ins.
    *
    * @pre \p pluginLib must have been loaded successfully.
    *
    * @param pluginLib The code entity (shared library, DLL, executable) to
    *                  validate.
    *
    * @throw vrkit::PluginInterfaceException
    *           Thrown if \p pluginLib is not a valid dynamic code entity for
    *           a vrkit plug-in.
    */
   static void basicValidation(const plugin::Module& module);

private:
   const plugin::Info mInfo;
};

}


#endif /* _VRKIT_ABSTRACT_PLUGIN_H_ */
