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

#ifndef INF_CONFIGURATION_H
#define INF_CONFIGURATION_H

#include <IOV/Config.h>

#include <string>
#include <vector>
#include <jccl/Config/Configuration.h>
#include <jccl/Config/ConfigElement.h>


namespace inf
{

/** Class for capturing all the configuration for the viewer app and system.
 *
 * This class holds a (set of) jccl::Configuration(s) for getting jccl style
 * configuration.  It could also be extended in the future to support other
 * configuration information that could be shared.
 */
class IOV_CLASS_API Configuration
{
public:
   /** Load the name configuration element file and adds it to the list of elts.
    */
   void loadConfigEltFile(const std::string& filename);

   /** Get the first configuration element found with the given name.
    * @return Returns a NULL shared ptr if not found.
    */
   jccl::ConfigElementPtr getConfigElement(const std::string& eltName);

   /** Return all the configuration elements named eltName.
    * @PARAM eltName  Name of the element type to look for.  If '', then return all elements.
    */
   std::vector<jccl::ConfigElementPtr> getAllConfigElements(const std::string& eltName = "");

protected:
   std::vector<jccl::Configuration> mElementLists;   /** List of element lists in configuration. */
};

}


#endif
