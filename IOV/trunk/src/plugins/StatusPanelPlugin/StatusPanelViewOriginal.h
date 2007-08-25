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

#ifndef _INF_STATUS_PANEL_VIEW_ORIGINAL_H_
#define _INF_STATUS_PANEL_VIEW_ORIGINAL_H_

#include <IOV/Config.h>

#include <vector>
#include <string>
#include <deque>
#include <map>

#include <OpenSG/OSGConfig.h>
#include <OpenSG/OSGNode.h>
#include <OpenSG/OSGGeometry.h>
#include <OpenSG/OSGColor.h>
#include <OpenSG/OSGClipPlaneChunk.h>

#include <IOV/UiBuilder.h>


namespace inf
{

class StatusPanel;

class StatusPanelViewOriginal
{
public:
   StatusPanelViewOriginal();

   typedef std::map<std::string, std::vector<std::string> > center_text_t;

   /** Initialize scene graph, fonts and everything else that is used. */
   void initialize(const float metersToAppUnits, StatusPanel* const panel);

   OSG::NodeRefPtr getPanelRoot()
   {
      return mRootPanelNode;
   }

   /** Set the panel to dirty.  Next update will rebuild. */
   void setDirty();

   /** Update the status panel if it is needed. */
   void update();

   void setWidthHeight(const float w, const float h,
                       const float borderWidth = 0.0f);

protected:
   void updatePanelScene();

   inf::UiBuilder        mBuilder;
   bool                  mIsDirty;     /**< When true, we need a rebuild of the panel. */

   OSG::NodeRefPtr       mRootPanelNode;
   OSG::NodeRefPtr       mPanelGeomNode;
   OSG::GeometryRefPtr   mPanelGeomCore;
   OSG::NodeRefPtr       mTextGeomNode;
   OSG::GeometryRefPtr   mTextGeomCore;

   inf::UiBuilder::Font* mFont;

   float mMetersToAppUnits;

   /**
    * Panel sizes are in OpenSG units.
    * Everything else is in normalized percentages or discrete num of's (i.e.,
    * num lines). Thus all real sizes are derived based on the panel size set.
    * This "should" make it possible to resize the panel and get good results.
    */
   //@{
   float    mPanWidth, mPanHeight;  /**< Panel size in OpenSG units. */
   float    mBorderWidth;           /**< Width of the border in real units. */
   float    mBorderDepth;           /**< Depth of the border in real units. */

   // -- Percentages -- //
   float    mTitleHeight;           /**< Height to make the titles. */

   float    mHeaderHeight;          /**< Size of the header section (percentage). */
   float    mStatusHeight;          /**< Size of the status section (percentage). */

   float    mStatusTextHeight;      /**< Fixed height (in OpenSG coords) of status text. */
   //@}

   // Colors/theming
   OSG::Color3f   mBgColor;         /**< Color of the background. */
   float          mBgAlpha;         /**< Alpha of the background */
   OSG::Color3f   mBorderColor;     /**< Color of the border */
   OSG::Color3f   mTitleColor;      /**< Color of the title */
   OSG::Color3f   mTextColor;       /**< Color of the text */

   bool           mDrawDebug;       /**< If true, draw debug geometry. */

   OSG::ClipPlaneChunkPtr mClipRight;
   OSG::ClipPlaneChunkPtr mClipBottom;

   StatusPanel* mStatusPanel;
};

}


#endif
