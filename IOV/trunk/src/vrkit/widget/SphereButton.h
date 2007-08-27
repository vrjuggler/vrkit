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

#ifndef _VRKIT_WIDGET_SPHERE_BUTTON_H_
#define _VRKIT_WIDGET_SPHERE_BUTTON_H_

#include <vrkit/Config.h>

#include <boost/signal.hpp>

#include <vrkit/scenedata/Event.h>
#include <vrkit/util/GeometryHighlightTraverser.h>
#include <vrkit/Widget.h>
#include <vrkit/widget/SphereButtonPtr.h>


namespace vrkit
{

namespace widget
{

/**
 * @note This class was moved into the vrkit::widget namespace in version 0.47.
 */
class VRKIT_CLASS_API SphereButton : public Widget
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
    * @return This object is returned as a shared pointer. A vrkit::WidgetPtr
    *         is returned rather than vrkit::FramePtr because these types do
    *         not allow for covariant return types in the method override.
    */
   virtual WidgetPtr init(const float metersToAppUnits);

   virtual void wandEntered();
   virtual void wandExited();
   virtual void wandPressed();
   virtual void wandReleased();
   virtual void wandMoved();

   // SIGNALS
   boost::signal<int (), event::ResultOperator> mClickedSignal;

protected:
   util::GeometryHighlightTraverser mGeomTraverser;
   unsigned int mHoverHighlightID;
   unsigned int mPressedHighlightID;
   unsigned int mActiveHighlightID;
   OSG::Color3f mIntersectColor;
   OSG::Color3f mGrabColor;
   virtual void updatePanelScene();
   bool mPressed;
};

}

}


#endif /* _VRKIT_WIDGET_SPHERE_BUTTON_H_ */
