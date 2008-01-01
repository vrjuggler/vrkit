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

#ifndef _VRKIT_WIDGET_FRAME_H_
#define _VRKIT_WIDGET_FRAME_H_

#include <vrkit/Config.h>

#include <OpenSG/OSGColor.h>

#include <vrkit/Widget.h>
#include <vrkit/widget/FramePtr.h>


namespace vrkit
{

namespace widget
{

/** \class Frame Frame.h vrkit/widget/Frame.h
 *
 * @note This class was moved into the vrkit::widget namespace in version 0.47.
 */
class VRKIT_CLASS_API Frame : public Widget
{
protected:
   Frame();

public:  // Configuration params //
   static FramePtr create()
   {
      return FramePtr(new Frame());
   }

   /**
    * Initializes this frame widget.
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

   virtual void setWidthHeight(const float w, const float h,
                               const float borderWidth = 0.0f);

protected:
   virtual void updatePanelScene();

protected:
   float    mBorderWidth;           /**< Width of the border in real units. */
   float    mBorderDepth;           /**< Depth of the border in real units. */

   // Colors/theming
   OSG::Color3f   mBorderColor;

   bool           mDrawDebug;    /**< If true, draw debug geometry. */
};

}

}


#endif /* _VRKIT_WIDGET_FRAME_H_ */
