#ifndef PLUGIN_H
#define PLUGIN_H

#include <jccl/Config/ConfigElementPtr.h>

#include <OpenSG/VRJ/Viewer/ViewerPtr.h>

#define INF_PLUGIN_API_MAJOR    1
#define INF_PLUGIN_API_MINOR    0

namespace inf
{

/**
 * A plugin is an abstract interface that allows for an extension point in the
 * system.  It is used to add capabilities to the application.
 */
class Plugin
{
public:
   virtual ~Plugin();

   virtual void init(inf::ViewerPtr viewer) = 0;

   virtual void update(inf::ViewerPtr viewer) = 0;

   virtual bool canHandleElement(jccl::ConfigElementPtr elt) = 0;

   /**
    * Configures this plug-in using the given config element.
    *
    * @pre \c canHandleElement() returned true.
    *
    * @return \c true is returned if this plug-in configured itself
    *         successfully, indicating that the given config element has
    *         been consumed.
    * @return \c false is returned if this plug-in was unable to configure
    *         itself using the given config element, indicating that the
    *         config element has not been consumed.
    */
   virtual bool config(jccl::ConfigElementPtr elt) = 0;

protected:
   Plugin();
};

}

#endif
