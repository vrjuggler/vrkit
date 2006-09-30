#include <IOV/Config.h>

#include <IOV/Viewer.h>
#include <IOV/User.h>

#include <IOV/Plugin/NavPlugin.h>


namespace inf
{

NavPlugin::NavPlugin()
{
   /* Do nothing. */ ;
}

NavPlugin::~NavPlugin()
{
   /* Do nothing. */ ;
}

void NavPlugin::update(inf::ViewerPtr viewer)
{
   updateNav(viewer, viewer->getUser()->getViewPlatform());
}

}
