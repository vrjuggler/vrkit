#ifndef _IOV_STATUS_PANEL_H_
#define _IOV_STATUS_PANEL_H_

#include <OpenSG/OSGConfig.h>
#include <OpenSG/OSGNode.h>
#include <OpenSG/OSGGeometry.h>
#include <OpenSG/OSGColor.h>

#include <IOV/UiBuilder.h>

#include <vpr/vprConfig.h>
#include <vpr/Util/Assert.h>

#include <vector>
#include <deque>


class StatusPanel
{
public:
   StatusPanel();

   /** Initialize scene graph, fonts and everything else that is used. */
   void initialize();

   OSG::NodeRefPtr getPanelRoot()
   { return mRootPanelNode; }

   void setHeaderTitle(std::string txt);
   void setCenterTitle(std::string txt);
   void setBottomTitle(std::string txt);

   /** The text for the header. */
   void setHeaderText(std::string header);

   /** Set text for the control section of the panel. */
   void setControlText(std::string text);

   /** Add another message to the status panel. */
   void addStatusMessage(std::string msg);

   void updatePanelScene();

protected:
   inf::UiBuilder        mBuilder;

   OSG::NodeRefPtr       mRootPanelNode;
   OSG::NodeRefPtr       mPanelGeomNode;
   OSG::GeometryRefPtr   mPanelGeomCore;
   OSG::NodeRefPtr       mTextGeomNode;
   OSG::GeometryRefPtr   mTextGeomCore;

   inf::UiBuilder::Font* mFont;

   std::string    mHeaderTitle;
   std::string    mCenterTitle;
   std::string    mBottomTitle;

   std::string    mHeaderText;
   std::string    mCenterText;
   std::deque<std::string> mStatusLines;

   /** Panel sizes are in OpenSG units.
    * Everything else is in normalized percentages or discrete num of's (ie. num lines).
    * Thus all real sizes are derived based on the panel size set.
    * This "should" make it possible to resize the panel and get good results.
    */
   float    mPanWidth, mPanHeight;  /**< Panel size in OpenSG units. */
   float    mBorderWidth;           /**< Width of the border in real units. */

   // -- Percentages -- //
   float    mTitleHeight;           /**< Height to make the titles. */

   float    mHeaderHeight;          /**< Size of the header section (percentage). */
   float    mStatusHeight;          /**< Size of the status section (percentage). */

   unsigned mStatusHistorySize;    /**< Number of status lines to keep around. */
   float    mStatusTextHeight;     /**< Fixed height (in OpenSG coords) of status text. */


   // Colors/theming
   OSG::Color3f   mBgColor;      /**< Color of the background. */
   float          mBgAlpha;
   OSG::Color3f   mBorderColor;
   OSG::Color3f   mTitleColor;
   OSG::Color3f   mTextColor;

};

#endif
