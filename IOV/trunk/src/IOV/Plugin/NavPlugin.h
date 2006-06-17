// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_NAV_PLUGIN_H_
#define _INF_NAV_PLUGIN_H_

#include <IOV/Config.h>

#include <IOV/ViewerPtr.h>
#include <IOV/Plugin.h>


namespace inf
{

class IOV_CLASS_API NavPlugin
   : public inf::Plugin
{
public:
   virtual ~NavPlugin();

   void update(inf::ViewerPtr viewer);

protected:
   NavPlugin();

   /**
    * Template method pattern interface used for navigation strategies to
    * update themselves.
    */
   virtual void updateNav(inf::ViewerPtr viewer,
                          inf::ViewPlatform& viewPlatform) = 0;
};


}


#endif
