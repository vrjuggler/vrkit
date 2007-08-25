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

#ifndef _INF_WIDGET_SPHERE_BUTTON_H_
#define _INF_WIDGET_SPHERE_BUTTON_H_

#include <IOV/Config.h>

#include <IOV/Event.h>
#include <IOV/Widget/Widget.h>
#include <IOV/Widget/SphereButtonPtr.h>
#include <IOV/GeometryHighlightTraverser.h>
#include <boost/signal.hpp>

namespace inf
{

class IOV_CLASS_API SphereButton : public Widget
{
protected:
   SphereButton();

public:
   virtual ~SphereButton();
   static SphereButtonPtr create()
   {
      return SphereButtonPtr(new SphereButton());
   }

   /**
    * Initializes this scroll bar widget.
    *
    * @param metersToAppUnits The conversion factor from meters (VR Juggler's
    *                         internal units) to the application-specific
    *                         units.
    *
    * @return This object is returned as a shared pointer. A inf::WidgetPtr is
    *         returned rather than inf::FramePtr because these types do not
    *         allow for covariant return types in the method override.
    */
   virtual WidgetPtr init(const float metersToAppUnits);

   virtual void wandEntered();
   virtual void wandExited();
   virtual void wandPressed();
   virtual void wandReleased();
   virtual void wandMoved();

   // SIGNALS
   boost::signal< int (), Event::ResultOperator > mClickedSignal;
protected:
   GeometryHighlightTraverser mGeomTraverser;
   unsigned int mHoverHighlightID;
   unsigned int mPressedHighlightID;
   unsigned int mActiveHighlightID;
   OSG::Color3f mIntersectColor;
   OSG::Color3f mGrabColor;
   virtual void updatePanelScene();
   bool mPressed;
};

}

#endif
