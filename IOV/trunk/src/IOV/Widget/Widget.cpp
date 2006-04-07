// Copyright (C) Infiscape Corporation 2005-2006

#include <IOV/Widget/Widget.h>

#include <OpenSG/OSGSimpleAttachments.h>

namespace inf
{

Widget::Widget()
   : mIsDirty(true), mMetersToAppUnits(0.0f), mWidth(0.0f), mHeight(0.0f),
     mBgColor(0.2f, 0.2f, 0.2f), mBgAlpha(0.6f), mDrawDebug(false)
{;}

Widget::~Widget()
{;}

void Widget::wandEntered()
{
   std::cout << "Wand Entered" << std::endl;
}

void Widget::wandExited()
{
   std::cout << "Wand Exited" << std::endl;
}

void Widget::wandPressed()
{
   std::cout << "Wand Pressed" << std::endl;
}

void Widget::wandReleased()
{
   std::cout << "Wand Released" << std::endl;
}

void Widget::wandMoved()
{
   static int i = 0;
   i++;
   std::cout << "Wand Moved: " << i << std::endl;
}

void Widget::init(const float metersToAppUnits)
{
   mMetersToAppUnits = metersToAppUnits;

   const float feet_to_app_units(0.3048f * mMetersToAppUnits);

   mWidth = 1.0f * feet_to_app_units;
   mHeight = 1.0f * feet_to_app_units;

   mRootWidgetNode = OSG::TransformNodePtr::create();
   mWidgetGeomNode = OSG::GeometryNodePtr::create();
   mWidgetGeomNode = mBuilder.createGeomGeo();

   OSG::setName(mRootWidgetNode.node(), "RootWidgetNode");
   OSG::setName(mWidgetGeomNode.node(), "WidgetGeomNode");
   OSG::setName(mWidgetGeomNode.core(), "WidgetGeomCore");

   OSG::CPEditor rpne(mRootWidgetNode.node());
   mRootWidgetNode.node()->addChild(mWidgetGeomNode);

   setDirty();
}

void Widget::setDirty()
{
   mIsDirty = true;
}

void Widget::update()
{
   if(mIsDirty)
   {
      updatePanelScene();
      mIsDirty = false;
   }
}

void Widget::setWidthHeight(const float w, const float h,
                            const float borderWidth)
{
   mWidth = w;
   mHeight = h;

   setDirty();
}

void Widget::move(const OSG::Pnt3f& pnt)
{
   OSG::beginEditCP(mRootWidgetNode.core(), OSG::Transform::MatrixFieldMask);
      mRootWidgetNode.core()->getMatrix().setTranslate(pnt);
   OSG::endEditCP(mRootWidgetNode.core(), OSG::Transform::MatrixFieldMask);
}

void Widget::moveTo(const OSG::Matrix& xform)
{
   OSG::beginEditCP(mRootWidgetNode.core(), OSG::Transform::MatrixFieldMask);
      mRootWidgetNode.core()->setMatrix(xform);
   OSG::endEditCP(mRootWidgetNode.core(), OSG::Transform::MatrixFieldMask);
}

void Widget::updatePanelScene()
{
   float radius = (mWidth < mHeight ? mWidth : mHeight);
   mBuilder.resetGeomGeo(mWidgetGeomNode.core());
   OSG::GeometryPtr new_geo = mBuilder.buildSphere(3, radius, mBgColor, mBgAlpha);
   mWidgetGeomNode = new_geo;
   OSG::setName(mWidgetGeomNode.core(), "WidgetGeomCore");
}

void Widget::setMaterial(OSG::MaterialPtr mat)
{
   if (OSG::NullFC == mat)
   {
      mWidgetGeomNode->setMaterial(mBuilder.createDefaultMaterial());
   }
   else
   {
      mWidgetGeomNode->setMaterial(mat);
   }
}

} // namespace inf

