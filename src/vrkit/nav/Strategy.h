// vrkit is (C) Copyright 2005-2008
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

#ifndef _VRKIT_NAV_STRATEGY_H_
#define _VRKIT_NAV_STRATEGY_H_

#include <vrkit/Config.h>

#include <vrkit/ViewerPtr.h>
#include <vrkit/viewer/Plugin.h>


namespace vrkit
{

namespace nav
{

/** \class Strategy Strategy.h vrkit/nav/Strategy.h
 *
 * @note This class was renamed from vrkit::NavPlugin in version 0.47.
 *
 * @todo This class should work in the same manner as vrkit::grab::Strategy or
 *       vrkit::isect::Strategy. Namely, there should be a single navigation
 *       plug-in that handles navigation strategies. However, this vision hsa
 *       not yet been fully realized.
 */
class VRKIT_CLASS_API Strategy
   : public viewer::Plugin
{
protected:
   Strategy(const plugin::Info& info);

public:
   virtual ~Strategy();

   void update(ViewerPtr viewer);

protected:
   /**
    * Template method pattern interface used for navigation strategies to
    * update themselves.
    */
   virtual void updateNav(ViewerPtr viewer, ViewPlatform& viewPlatform) = 0;
};

}

}


#endif /* _VRKIT_NAV_STRATEGY_H_ */
