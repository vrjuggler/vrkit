// Copyright (C) Infiscape Corporation 2005-2007

#include <IOV/Widget/ScrollBar.h>
#include <IOV/Widget/SphereButton.h>
#include <OpenSG/OSGSimpleAttachments.h>

#include <boost/bind.hpp>

namespace inf
{

ScrollBar::ScrollBar() : mValue(0), mMinValue(0), mMaxValue(100), mIncrement(10)
{;}

ScrollBar::~ScrollBar()
{;}

WidgetPtr ScrollBar::init(const float metersToAppUnits)
{
   WidgetPtr myself = Widget::init(metersToAppUnits);

   // Add buttons
   mUpButton = SphereButton::create();
   mUpButton->init(mMetersToAppUnits);
   mDownButton = SphereButton::create();
   mDownButton->init(mMetersToAppUnits);
   mCurrentButton = SphereButton::create();
   mCurrentButton->init(mMetersToAppUnits);

   addChild(mUpButton);
   addChild(mDownButton);
   addChild(mCurrentButton);

   // Add buttons to OpenSG graph.
   OSG::beginEditCP(mTransformNode.node());
      mTransformNode.node()->addChild(mUpButton->getRoot());
      mTransformNode.node()->addChild(mDownButton->getRoot());
      mTransformNode.node()->addChild(mCurrentButton->getRoot());
   OSG::endEditCP(mTransformNode.node());

   mUpButton->mClickedSignal.connect(boost::bind(&ScrollBar::onButtonClicked, this, true));
   mDownButton->mClickedSignal.connect(boost::bind(&ScrollBar::onButtonClicked, this, false));

   return myself;
}

Event::ResultType ScrollBar::onButtonClicked(bool upButton)
{
   if (upButton && mValue < mMaxValue)
   {
      mValue += mIncrement;
      mValue = (mValue > mMaxValue ? mMaxValue : mValue);
   }
   else if (!upButton && mValue > mMinValue)
   {
      mValue -= mIncrement;
      mValue = (mValue < mMinValue ? mMinValue : mValue);
   }
   std::cout << "Val: " << mValue << std::endl;
   updateCurPos();

   // Send signal
   mValueChangedSignal(mValue);

   return Event::CONTINUE;
}

void ScrollBar::updateCurPos()
{
   float pct = (float)mValue/(float)mMaxValue;
   float padding = mWidth * 2.0f;
   float pos = padding + (mHeight-padding*2)*pct;
   mCurrentButton->move(OSG::Pnt3f(mWidth/2, pos, 0.0f));
}

void ScrollBar::updatePanelScene()
{
   mBuilder.resetGeomGeo(mWidgetGeomNode.core());

   // Draw the panel outline
   OSG::Vec2f panel_ll(0,0), panel_ul(0, mHeight), panel_ur(mWidth, mHeight), panel_rl(mWidth,0);
   mBuilder.buildRectangleOutline(mWidgetGeomNode.core(), mBgColor, panel_ll, panel_ur, 0.2);
   OSG::setName(mWidgetGeomNode.core(), "ScrollBarGeomCore");
}

void ScrollBar::setWidthHeight(const float w, const float h, const float borderWidth)
{
   Widget::setWidthHeight(w, h, borderWidth);

   mUpButton->move(OSG::Pnt3f(mWidth/2, mHeight, 0.0f));
   mDownButton->move(OSG::Pnt3f(mWidth/2, 0.0f, 0.0f));
   mCurrentButton->move(OSG::Pnt3f(mWidth/2, mHeight/2, 0.0f));

   mUpButton->setWidthHeight(mWidth, mWidth, 0.0f);
   mUpButton->update();
   mDownButton->setWidthHeight(mWidth, mWidth, 0.0f);
   mDownButton->update();
   mCurrentButton->setWidthHeight(mWidth, mWidth, 0.0f);
   mCurrentButton->update();

   updateCurPos();
}
void ScrollBar::wandEntered()
{
}

void ScrollBar::wandExited()
{
}

void ScrollBar::wandPressed()
{
}

void ScrollBar::wandReleased()
{
}

void ScrollBar::wandMoved()
{
}

} // namespace inf

