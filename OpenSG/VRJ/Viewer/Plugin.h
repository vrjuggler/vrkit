#ifndef PLUGIN_H
#define PLUGIN_H

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

protected:
   Plugin();
};

}

#endif
