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
