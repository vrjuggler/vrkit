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

#include <OpenSG/OSGSimpleAttachments.h>

#include <vrkit/widget/SphereButton.h>


namespace vrkit
{

namespace widget
{

SphereButton::SphereButton()
   : mActiveHighlightID(0)
   , mPressed(false)
{
   /* Do nothing. */ ;
}

SphereButton::~SphereButton()
{
   /* Do nothing. */ ;
}

WidgetPtr SphereButton::init(const float metersToAppUnits)
{
   WidgetPtr myself = Widget::init(metersToAppUnits);

   mIntersectColor = OSG::Color3f(1.0f, 0.0, 0.0f);
   mGrabColor = OSG::Color3f(0.0f, 1.0, 0.0f);
   mHoverHighlightID = mGeomTraverser.createScribeMaterial(false, GL_LINE,
                                                           true, false,
                                                           false, 0.05f,
                                                           1.0f,
                                                           mIntersectColor);
   mPressedHighlightID = mGeomTraverser.createScribeMaterial(false, GL_LINE,
                                                             true, false,
                                                             false, 0.05f,
                                                             1.0f,
                                                             mGrabColor);

   return myself;
}

void SphereButton::updatePanelScene()
{
   float radius = (mWidth < mHeight ? mWidth : mHeight);
   mBuilder.resetGeomGeo(mWidgetGeomNode.core());
   OSG::GeometryPtr new_geo = mBuilder.buildSphere(3, radius, mBgColor,
                                                   mBgAlpha);
   mWidgetGeomNode = new_geo;
   OSG::setName(mWidgetGeomNode.core(), "WidgetGeomCore");
}

void SphereButton::wandEntered()
{
   mGeomTraverser.addHighlightMaterial(mRootWidgetNode.node(),
                                       mHoverHighlightID);

/*
   if (mActiveHighlightID == 0)
   {
      mGeomTraverser.addHighlightMaterial(mRootWidgetNode.node(),
                                          mHoverHighlightID);
      mActiveHighlightID = mHoverHighlightID;
   }
*/
}

void SphereButton::wandExited()
{
   mGeomTraverser.removeHighlightMaterial(mRootWidgetNode.node(),
                                          mHoverHighlightID);
/*
   if (!mPressed)
   {
      mGeomTraverser.removeHighlightMaterial(mRootWidgetNode.node(),
                                             mHoverHighlightID);
      mActiveHighlightID = 0;
   }
*/
}

void SphereButton::wandPressed()
{
   mPressed = true;
/*
   mGeomTraverser.swapHighlightMaterial(mRootWidgetNode.node(),
                                        mHoverHighlightID,
                                        mPressedHighlightID);
   mActiveHighlightID = mPressedHighlightID;
*/
   mClickedSignal();
}

void SphereButton::wandReleased()
{
   mPressed = false;
/*
   if (mActiveHighlightID != 0)
   {
      mGeomTraverser.removeHighlightMaterial(mRootWidgetNode.node(),
                                             mActiveHighlightID);
      mActiveHighlightID = 0;
   }
*/
}

void SphereButton::wandMoved()
{
}

} // namespace widget

} // namespace vrkit
