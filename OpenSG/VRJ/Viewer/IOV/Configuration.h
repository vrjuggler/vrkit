#ifndef INF_CONFIGURATION_H
#define INF_CONFIGURATION_H

#include <jccl/Config/Configuration.h>
#include <jccl/Config/ConfigElement.h>
#include <string>
#include <vector>

namespace inf
{

/** Class for capturing all the configuration for the viewer app and system.
 *
 * This class holds a (set of) jccl::Configuration(s) for getting jccl style
 * configuration.  It could also be extended in the future to support other
 * configuration information that could be shared.
 */
class Configuration
{
public:
   /** Load the name configuration element file and adds it to the list of elts.
    */
   void loadConfigEltFile(std::string filename);

   /** Get the first configuration element found with the given name.
    * @return Returns a NULL shared ptr if not found.
    */
   jccl::ConfigElementPtr getConfigElement(std::string eltName);

   /** Return all the configuration elements named eltName.
    * @PARAM eltName  Name of the element type to look for.  If '', then return all elements.
    */
   std::vector<jccl::ConfigElementPtr> getAllConfigElements(std::string eltName="");

protected:
   std::vector<jccl::Configuration> mElementLists;   /** List of element lists in configuration. */
};

}
#endif
