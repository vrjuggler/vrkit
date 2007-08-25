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

#include <IOV/Config.h>

#include <IOV/ViewerPtr.h>
#include <IOV/Plugin.h>


namespace inf
{

class IOV_CLASS_API NavPlugin
   : public inf::Plugin
{
protected:
   NavPlugin(const inf::plugin::Info& info);

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
