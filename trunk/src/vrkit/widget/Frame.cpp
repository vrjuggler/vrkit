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

#include <vrkit/widget/Frame.h>


namespace vrkit
{

namespace widget
{

Frame::Frame()
{
   /* Do nothing. */ ;
}

WidgetPtr Frame::init(const float metersToAppUnits)
{
   WidgetPtr myself = Widget::init(metersToAppUnits);

   const float feet_to_app_units(0.3048f * mMetersToAppUnits);
   mBorderWidth = 0.4f * feet_to_app_units;
   mBorderDepth = 0.2f * feet_to_app_units;
   mBorderColor.setValuesRGB(1,1,1);

   return myself;
}

void Frame::setWidthHeight(const float w, const float h, const float borderWidth)
{
   Widget::setWidthHeight(w, h, borderWidth);

   mBorderWidth = borderWidth;
   if(0.0f == borderWidth)
   {
      mBorderWidth = ((mWidth+mHeight)/2.0f)/20.0f;
   }
   mBorderDepth = mBorderWidth/2.0f;

   setDirty();
}

void Frame::updatePanelScene()
{
   mBuilder.resetGeomGeo(mWidgetGeomNode.core());

   // Draw the panel outline
   OSG::Vec2f panel_ll(0,0), panel_ul(0, mHeight), panel_ur(mWidth, mHeight), panel_rl(mWidth,0);

   const float feet_to_app_units(0.3048f * mMetersToAppUnits);

   const float inner_rad(0.2f * feet_to_app_units);
   unsigned int num_segs(8);
   const float front_depth( mBorderDepth/2.0f);
   const float back_depth (-mBorderDepth/2.0f);
   //const float bg_depth(-0.1f * feet_to_app_units);

   mBuilder.buildRoundedRectangle(mWidgetGeomNode.core(), mBorderColor, panel_ll, panel_ur, inner_rad, inner_rad+mBorderWidth,
                                 num_segs, false, front_depth, back_depth, 1.0);
   mBuilder.buildRoundedRectangle(mWidgetGeomNode.core(), mBgColor,     panel_ll, panel_ur, 0.0, inner_rad+(mBorderWidth*1.5f),
                                 num_segs, true,  back_depth,    back_depth, mBgAlpha);

   OSG::Color3f dbg_color(1,0,0);

   // --- Draw debug outlines --- //
   /*
   if(mDrawDebug)
   {
      mBuilder.buildRectangleOutline(mWidgetGeomNode.core(), dbg_color, panel_ll, panel_ur, 0.2);
      mBuilder.buildRectangleOutline(mWidgetGeomNOde.core(), OSG::Color3f(1,1,0),
                                                             OSG::Vec2f(0,header_title_ul.y()-total_header_height),
                                                             OSG::Vec2f(mWidth, header_title_ul.y()), 0.3);       // Header
      mBuilder.buildRectangleOutline(mWidgetGeomNode.core(), OSG::Color3f(0,1,0),
                                                             OSG::Vec2f(0,center_title_ul.y()-total_center_height),
                                                             OSG::Vec2f(mWidth, center_title_ul.y()), 0.4);       // Center
      mBuilder.buildRectangleOutline(mWidgetGeomNode.core(), OSG::Color3f(0,0,1),
                                                             OSG::Vec2f(0,status_title_ul.y()-total_status_height),
                                                             OSG::Vec2f(mWidth, status_title_ul.y()), 0.3);       // Status
   }
   */
}

} // namespace widget

} // namespace vrkit
