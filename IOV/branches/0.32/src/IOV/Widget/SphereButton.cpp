// Copyright (C) Infiscape Corporation 2005-2007

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

   
   OSG::setName(mTransformNode.node(), "SphereRootNode");
   OSG::setName(mWidgetGeomNode.node(), "SphereGeomNode");
   OSG::setName(mWidgetGeomNode.core(), "SphereGeomCore");

   return myself;
}

void SphereButton::updatePanelScene()
{
   float radius = (mWidth < mHeight ? mWidth : mHeight);
   mBuilder.resetGeomGeo(mWidgetGeomNode.core());
   OSG::GeometryPtr new_geo = mBuilder.buildSphere(3, radius, mBgColor, mBgAlpha);
   mWidgetGeomNode = new_geo;
   OSG::setName(mWidgetGeomNode.core(), "SphereGeomCore");
}

void SphereButton::wandEntered()
{
   std::cout << "SphereButton::wandEntered" << std::endl;
   mGeomTraverser.addHighlightMaterial(mTransformNode.node(), mHoverHighlightID);

/*
   if (mActiveHighlightID == 0)
   {
      mGeomTraverser.addHighlightMaterial(mTransformNode.node(), mHoverHighlightID);
      mActiveHighlightID = mHoverHighlightID;
   }
*/
}

void SphereButton::wandExited()
{
   mGeomTraverser.removeHighlightMaterial(mTransformNode.node(), mHoverHighlightID);
/*
   if (!mPressed)
   {
      mGeomTraverser.removeHighlightMaterial(mTransformNode.node(), mHoverHighlightID);
      mActiveHighlightID = 0;
   }
*/
}

void SphereButton::wandPressed()
{
   mPressed = true;
/*
   mGeomTraverser.swapHighlightMaterial(mTransformNode.node(), mHoverHighlightID,
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
      mGeomTraverser.removeHighlightMaterial(mTransformNode.node(), mActiveHighlightID);
      mActiveHighlightID = 0;
   }
*/
}

void SphereButton::wandMoved()
{
}

} // namespace inf

