#ifndef _INF_NAV_PLUGIN_H_
#define _INF_NAV_PLUGIN_H_

#include <IOV/ViewerPtr.h>
#include <IOV/Plugin.h>


namespace inf
{

class NavPlugin : public inf::Plugin
{
public:
   virtual ~NavPlugin()
   {
      /* Do nothing. */ ;
   }

   void updateState(inf::ViewerPtr viewer)
   {
      updateNavState(viewer, viewer->getUser()->getViewPlatform());
   }

   void run(inf::ViewerPtr viewer)
   {
      runNav(viewer, viewer->getUser()->getViewPlatform());
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
   virtual void updateNavState(inf::ViewerPtr viewer,
                               inf::ViewPlatform& viewPlatform) = 0;

   virtual void runNav(inf::ViewerPtr viewer,
                       inf::ViewPlatform& viewPlatform) = 0;
};


}


#endif
