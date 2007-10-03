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

#ifndef _VRKIT_WIDGET_SCROLL_BAR_H_
#define _VRKIT_WIDGET_SCROLL_BAR_H_

#include <vrkit/Config.h>

#include <boost/signal.hpp>

#include <vrkit/Widget.h>
#include <vrkit/widget/ScrollBarPtr.h>
#include <vrkit/widget/SphereButtonPtr.h>


namespace vrkit
{

namespace widget
{

/** \class ScrollBar ScrollBar.h vrkit/widget/ScrollBar.h
 *
 * @note This class was moved into the vrkit::widget namespace in version 0.47.
 */
class VRKIT_CLASS_API ScrollBar : public Widget
{
protected:
   ScrollBar();

public:
   virtual ~ScrollBar();

   static ScrollBarPtr create()
   {
      return ScrollBarPtr(new ScrollBar());
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

   virtual void setWidthHeight(const float w, const float h,
                               const float borderWidth);

   void updateCurPos();

   void setValue(const int val)
   {
      if (val < mMinValue)
      {
         mValue = mMinValue;
      }
      else if (val > mMaxValue)
      {
         mValue = mMaxValue;
      }
      else
      {
         mValue = val;
      }
      updateCurPos();
   }

   int value() const
   {
      return mValue;
   }

   void setMaxValue(const int val)
   {
      mMaxValue = val;
      if (mValue > mMaxValue)
      {
         mValue = mMaxValue;
      }
      updateCurPos();
   }

   int maxValue() const
   {
      return mMaxValue;
   }

   void setMinValue(const int val)
   {
      mMinValue = val;
      if (mValue < mMinValue)
      {
         mValue = mMinValue;
      }
      updateCurPos();
   }

   int minValue() const
   {
      return mMinValue;
   }

   void setIncrement(const int inc)
   {
      mIncrement = inc;
   }

   int getIncrement() const
   {
      return mIncrement;
   }

   event::ResultType onButtonClicked(bool upButton);

   virtual void wandEntered();
   virtual void wandExited();
   virtual void wandPressed();
   virtual void wandReleased();
   virtual void wandMoved();

   // SIGNALS
   boost::signal<int (const int), event::ResultOperator> mValueChangedSignal;

protected:
   SphereButtonPtr mUpButton;
   SphereButtonPtr mDownButton;
   SphereButtonPtr mCurrentButton;
   int mValue;
   int mMinValue;
   int mMaxValue;
   int mIncrement;

   virtual void updatePanelScene();
};

}

}


#endif /* _VRKIT_WIDGET_SCROLL_BAR_H_ */
