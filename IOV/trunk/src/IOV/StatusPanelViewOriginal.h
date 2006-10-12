#ifndef _INF_STATUS_PANEL_VIEW_H_
#define _INF_STATUS_PANEL_VIEW_H_

#include <OpenSG/OSGConfig.h>
#include <OpenSG/OSGNode.h>
#include <OpenSG/OSGGeometry.h>
#include <OpenSG/OSGColor.h>
#include <OpenSG/OSGClipPlaneChunk.h>

#include <IOV/UiBuilder.h>
#include <IOV/Config.h>
#include <IOV/StatusPanel.h>

#include <vector>
#include <string>
#include <deque>
#include <map>

namespace inf
{
   
class IOV_CLASS_API StatusPanelView
{   
public:
   
   StatusPanelView();
   
   typedef std::map<std::vector<int>, std::vector<std::string> > center_text_t;
   
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
   
   void setWidthHeight(const float w, const float h, const float borderWidth=0.0f);

protected:
   
   void updatePanelScene();
   
protected:
         
   inf::UiBuilder        mBuilder;
   bool                  mIsDirty;     /**< When true, we need a rebuild of the panel. */

   OSG::NodeRefPtr       mRootPanelNode;
   OSG::NodeRefPtr       mPanelGeomNode;
   OSG::GeometryRefPtr   mPanelGeomCore;
   OSG::NodeRefPtr       mTextGeomNode;
   OSG::GeometryRefPtr   mTextGeomCore;

   inf::UiBuilder::Font* mFont;
   
   float mMetersToAppUnits;

   /** Panel sizes are in OpenSG units.
    * Everything else is in normalized percentages or discrete num of's (ie. num lines).
    * Thus all real sizes are derived based on the panel size set.
    * This "should" make it possible to resize the panel and get good results.
    */
   float    mPanWidth, mPanHeight;  /**< Panel size in OpenSG units. */
   float    mBorderWidth;           /**< Width of the border in real units. */
   float    mBorderDepth;           /**< Depth of the border in real units. */

   // -- Percentages -- //
   float    mTitleHeight;           /**< Height to make the titles. */

   float    mHeaderHeight;          /**< Size of the header section (percentage). */
   float    mStatusHeight;          /**< Size of the status section (percentage). */

   float    mStatusTextHeight;     /**< Fixed height (in OpenSG coords) of status text. */


   // Colors/theming
   OSG::Color3f   mBgColor;      /**< Color of the background. */
   float          mBgAlpha;
   OSG::Color3f   mBorderColor;
   OSG::Color3f   mTitleColor;
   OSG::Color3f   mTextColor;

   bool           mDrawDebug;    /**< If true, draw debug geometry. */

   OSG::ClipPlaneChunkPtr mClipRight;
   OSG::ClipPlaneChunkPtr mClipBottom;
   
   StatusPanel* mStatusPanel;
};
   
}

#endif
