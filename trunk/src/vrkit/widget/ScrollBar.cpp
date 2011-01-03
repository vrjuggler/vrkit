// vrkit is (C) Copyright 2005-2011
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

#include <boost/bind.hpp>

#include <OpenSG/OSGSimpleAttachments.h>

#include <vrkit/widget/SphereButton.h>
#include <vrkit/widget/ScrollBar.h>


namespace vrkit
{

namespace widget
{

ScrollBar::ScrollBar()
   : mValue(0)
   , mMinValue(0)
   , mMaxValue(100)
   , mIncrement(10)
{
   /* Do nothing. */ ;
}

ScrollBar::~ScrollBar()
{
   /* Do nothing. */ ;
}

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
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor rwne(mRootWidgetNode.node(), OSG::Node::ChildrenFieldMask);
#endif
   mRootWidgetNode.node()->addChild(mUpButton->getRoot());
   mRootWidgetNode.node()->addChild(mDownButton->getRoot());
   mRootWidgetNode.node()->addChild(mCurrentButton->getRoot());

   mUpButton->mClickedSignal.connect(
      boost::bind(&ScrollBar::onButtonClicked, this, true)
   );
   mDownButton->mClickedSignal.connect(
      boost::bind(&ScrollBar::onButtonClicked, this, false)
   );

   return myself;
}

event::ResultType ScrollBar::onButtonClicked(bool upButton)
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

   return event::CONTINUE;
}

void ScrollBar::updateCurPos()
{
   const float pct = static_cast<float>(mValue) / static_cast<float>(mMaxValue);
   const float padding = mWidth * 2.0f;
   const float pos = padding + (mHeight - padding * 2) * pct;
   mCurrentButton->move(OSG::Pnt3f(mWidth / 2, pos, 0.0f));
}

void ScrollBar::updatePanelScene()
{
   mBuilder.resetGeomGeo(mWidgetGeomNode.core());

   // Draw the panel outline
   OSG::Vec2f panel_ll(0,0), panel_ul(0, mHeight), panel_ur(mWidth, mHeight), panel_rl(mWidth,0);
   mBuilder.buildRectangleOutline(mWidgetGeomNode.core(), mBgColor, panel_ll, panel_ur, 0.2);
   OSG::setName(mWidgetGeomNode.core(), "ScrollBarGeomCore");
}

void ScrollBar::setWidthHeight(const float w, const float h,
                               const float borderWidth)
{
   Widget::setWidthHeight(w, h, borderWidth);

   mUpButton->move(OSG::Pnt3f(mWidth / 2, mHeight, 0.0f));
   mDownButton->move(OSG::Pnt3f(mWidth / 2, 0.0f, 0.0f));
   mCurrentButton->move(OSG::Pnt3f(mWidth / 2, mHeight / 2, 0.0f));

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

} // namespace widget

} // namespace vrkit
