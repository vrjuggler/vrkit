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

#include <OpenSG/OSGSimpleAttachments.h>

#include <vrkit/Widget.h>


namespace vrkit
{

Widget::Widget()
   : mIsDirty(true)
   , mMetersToAppUnits(0.0f)
   , mWidth(0.0f)
   , mHeight(0.0f)
   , mBgColor(0.2f, 0.2f, 0.2f)
   , mBgAlpha(0.6f)
   , mDrawDebug(false)
{
   /* Do nothing. */ ;
}

Widget::~Widget()
{
   /* Do nothing. */ ;
}

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

WidgetPtr Widget::init(const float metersToAppUnits)
{
   mMetersToAppUnits = metersToAppUnits;

   const float feet_to_app_units(0.3048f * mMetersToAppUnits);

   mWidth = 1.0f * feet_to_app_units;
   mHeight = 1.0f * feet_to_app_units;

   mRootWidgetNode = OSG::TransformNodePtr::create();
   mWidgetGeomNode = OSG::GeometryNodePtr::create();
   mWidgetGeomNode = mBuilder.createGeomGeo();

   OSG::UInt32 trav_mask = mRootWidgetNode.node()->getTravMask();
   trav_mask = (trav_mask & ~SceneObject::ISECT_MASK);
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor rwne(
      mRootWidgetNode.node(),
      OSG::Node::TravMaskFieldMask | OSG::Node::ChildrenFieldMask
   );
#endif
   mRootWidgetNode.node()->setTravMask(trav_mask);

   OSG::setName(mRootWidgetNode.node(), "RootWidgetNode");
   OSG::setName(mWidgetGeomNode.node(), "WidgetGeomNode");
   OSG::setName(mWidgetGeomNode.core(), "WidgetGeomCore");

   mRootWidgetNode.node()->addChild(mWidgetGeomNode);

   setDirty();

   return boost::dynamic_pointer_cast<Widget>(shared_from_this());
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
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor rwne(mRootWidgetNode.core(), OSG::Transform::MatrixFieldMask);
   mRootWidgetNode.core()->getMatrix().setTranslate(pnt);
#else
   mRootWidgetNode.core()->editMatrix().setTranslate(pnt);
#endif
}

void Widget::moveTo(const OSG::Matrix& xform)
{
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor rwne(mRootWidgetNode.core(), OSG::Transform::MatrixFieldMask);
#endif
   mRootWidgetNode.core()->setMatrix(xform);
}

void Widget::updatePanelScene()
{
   float radius = (mWidth < mHeight ? mWidth : mHeight);
   mBuilder.resetGeomGeo(mWidgetGeomNode.core());
   OSG::GeometryPtr new_geo = mBuilder.buildSphere(3, radius, mBgColor,
                                                   mBgAlpha);
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

} // namespace vrkit
