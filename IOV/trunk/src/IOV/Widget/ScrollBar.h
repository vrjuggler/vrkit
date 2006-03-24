// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_WIDGET_SCROLL_BAR_H_
#define _INF_WIDGET_SCROLL_BAR_H_

#include <IOV/Config.h>

#include <IOV/Widget/Widget.h>
#include <IOV/Widget/ScrollBarPtr.h>
#include <IOV/Widget/SphereButtonPtr.h>
#include <boost/signal.hpp>

namespace inf
{

class IOV_CLASS_API ScrollBar : public Widget
{
protected:
   ScrollBar();

public:
   virtual ~ScrollBar();
   static ScrollBarPtr create()
   {
      return ScrollBarPtr(new ScrollBar());
   }

   virtual void init(const float metersToAppUnits);
   virtual void setWidthHeight(const float w, const float h, const float borderWidth);

   void updateCurPos();

   void setValue(const int val)
   {
      if (val < mMinValue)
      { mValue = mMinValue; }
      else if (val > mMaxValue)
      { mValue = mMaxValue; }
      else
      { mValue = val; }
      updateCurPos();
   }

   int value()
   { return mValue; }

   void setMaxValue(const int val)
   {
      mMaxValue = val;
      if (mValue > mMaxValue)
      { mValue = mMaxValue; }
      updateCurPos();
   }

   int maxValue()
   { return mMaxValue; }

   void setMinValue(const int val)
   {
      mMinValue = val;
      if (mValue < mMinValue)
      { mValue = mMinValue; }
      updateCurPos();
   }

   int minValue()
   { return mMinValue; }
   
   void setIncrement(const int inc)
   { mIncrement = inc; }

   int setIncrement()
   { return mIncrement; }

   Event::ResultType onButtonClicked(bool upButton);

   virtual void wandEntered();
   virtual void wandExited();
   virtual void wandPressed();
   virtual void wandReleased();
   virtual void wandMoved();

   // SIGNALS
   boost::signal< int (const int), Event::ResultOperator > mValueChangedSignal;
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

#endif

