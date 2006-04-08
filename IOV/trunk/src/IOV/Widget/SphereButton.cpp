// Copyright (C) Infiscape Corporation 2005-2006

#include <IOV/Widget/SphereButton.h>
#include <OpenSG/OSGSimpleAttachments.h>

namespace inf
{

SphereButton::SphereButton() : mActiveHighlightID(0), mPressed(false)
{;}

SphereButton::~SphereButton()
{;}

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
   mPressedHighlightID  = mGeomTraverser.createScribeMaterial(false, GL_LINE,
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
   OSG::GeometryPtr new_geo = mBuilder.buildSphere(3, radius, mBgColor, mBgAlpha);
   mWidgetGeomNode = new_geo;
   OSG::setName(mWidgetGeomNode.core(), "WidgetGeomCore");
}

void SphereButton::wandEntered()
{
   mGeomTraverser.addHighlightMaterial(mRootWidgetNode.node(), mHoverHighlightID);

/*
   if (mActiveHighlightID == 0)
   {
      mGeomTraverser.addHighlightMaterial(mRootWidgetNode.node(), mHoverHighlightID);
      mActiveHighlightID = mHoverHighlightID;
   }
*/
}

void SphereButton::wandExited()
{
   mGeomTraverser.removeHighlightMaterial(mRootWidgetNode.node(), mHoverHighlightID);
/*
   if (!mPressed)
   {
      mGeomTraverser.removeHighlightMaterial(mRootWidgetNode.node(), mHoverHighlightID);
      mActiveHighlightID = 0;
   }
*/
}

void SphereButton::wandPressed()
{
   mPressed = true;
/*
   mGeomTraverser.swapHighlightMaterial(mRootWidgetNode.node(), mHoverHighlightID,
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
      mGeomTraverser.removeHighlightMaterial(mRootWidgetNode.node(), mActiveHighlightID);
      mActiveHighlightID = 0;
   }
*/
}

void SphereButton::wandMoved()
{
}

} // namespace inf

