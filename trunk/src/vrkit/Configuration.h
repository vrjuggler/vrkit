// vrkit is (C) Copyright 2005-2008
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

#ifndef _VRKIT_CONFIGURATION_H_
#define _VRKIT_CONFIGURATION_H_

#include <vrkit/Config.h>

#include <string>
#include <vector>
#include <jccl/Config/Configuration.h>
#include <jccl/Config/ConfigElementPtr.h>


namespace vrkit
{

/** \class Configuration Configuration.h vrkit/Configuration.h
 *
 * Class for capturing all the configuration for the viewer app and system.
 *
 * This class holds a (set of) jccl::Configuration(s) for getting JCCL-style
 * configuration.  It could also be extended in the future to support other
 * configuration information that could be shared.
 */
class VRKIT_CLASS_API Configuration
{
public:
   /**
    * Loads the named configuration element file and adds it to the list of
    * elts.
    *
    * @param filename The Juggler configuration file to load.
    *
    * @throw vrkit::ConfigurationException
    *           Thrown if the named file cannot be loaded.
    *
    * @note This method was changed in version 0.50.1 to throw an exception
    *       when loading a configuration file fails.
    */
   void loadConfigEltFile(const std::string& filename);

   /**
    * Gets the first configuration element found with the given name.
    *
    * @return Returns a NULL shared ptr if not found.
    */
   jccl::ConfigElementPtr getConfigElement(const std::string& eltName);

   /**
    * Returns all the configuration elements with the given name.
    *
    * @param eltName The name of the element type to find. If an empty string
    *                is given, then all elements are returned.
    */
   std::vector<jccl::ConfigElementPtr>
      getAllConfigElements(const std::string& eltName = "");

private:
   /** List of element lists in configuration. */
   std::vector<jccl::Configuration> mElementLists;
};

}


#endif /* _VRKIT_CONFIGURATION_H_ */
