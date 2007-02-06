// Copyright (C) Infiscape Corporation 2005-2007

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
protected:
   NavPlugin();

public:
   virtual ~NavPlugin();

   void update(inf::ViewerPtr viewer);

protected:
   /**
    * Template method pattern interface used for navigation strategies to
    * update themselves.
    */
   virtual void updateNav(inf::ViewerPtr viewer,
                          inf::ViewPlatform& viewPlatform) = 0;
};


}


#endif
