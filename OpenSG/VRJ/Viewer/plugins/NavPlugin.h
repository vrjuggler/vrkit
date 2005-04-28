#ifndef _INF_NAV_PLUGIN_H_
#define _INF_NAV_PLUGIN_H_

#include <OpenSG/VRJ/Viewer/IOV/ViewerPtr.h>
#include <OpenSG/VRJ/Viewer/IOV/Plugin.h>


namespace inf
{

class NavPlugin : public inf::Plugin
{
public:
   virtual ~NavPlugin()
   {
      /* Do nothing. */ ;
   }

   void update(inf::ViewerPtr viewer)
   {
      updateNav(viewer, viewer->getUser()->getViewPlatform());
   }

protected:
   NavPlugin()
   {
      /* Do nothing. */ ;
   }

   /**
    * Template method pattern interface used for navigation strategies to
    * update themselves.
    */
   virtual void updateNav(inf::ViewerPtr viewer,
                          inf::ViewPlatform& viewPlatform) = 0;
};


}


#endif
