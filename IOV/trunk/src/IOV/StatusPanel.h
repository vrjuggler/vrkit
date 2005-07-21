// Copyright (C) Infiscape Corporation 2005

#ifndef _IOV_STATUS_PANEL_H_
#define _IOV_STATUS_PANEL_H_

#include <IOV/Config.h>

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

class IOV_CLASS_API StatusPanel
{
public:
   enum ControlTextLine
   {
      LINE1 = 0,
      LINE2 = LINE1 + 1,
      LINE3 = LINE2 + 1,
      LINE4 = LINE3 + 1,
      LINE5 = LINE4 + 1,
      END
   };

   StatusPanel(const float metersToAppUnits);

   /** Initialize scene graph, fonts and everything else that is used. */
   void initialize();

   OSG::NodeRefPtr getPanelRoot()
   { return mRootPanelNode; }

   void setHeaderTitle(const std::string& txt);
   void setCenterTitle(const std::string& txt);
   void setBottomTitle(const std::string& txt);

   /** The text for the header. */
   void setHeaderText(const std::string& header);

   /**
    * Sets text for the named line of the panel's control section.  This
    * overwrites whatever was previously in the buffer for the named line.
    *
    * @since 0.6.0
    */
   void setControlText(const ControlTextLine line, const std::string& text);

   /**
    * Adds the given text to the named line of the panel's control section.
    *
    * @since 0.6.0
    */
   void addControlText(const ControlTextLine line, const std::string& text,
                       const unsigned int priority = 1);

   /**
    * Removes the given text from the named line of the panel's control
    * section.  If the given text is not in the named line, this has no
    * effect.
    *
    * @since 0.6.0
    */
   void removeControlText(const ControlTextLine line, const std::string& text);

   /**
    * Determines if the named line contains the given text.
    *
    * @since 0.6.0
    */
   bool hasControlText(const ControlTextLine line, const std::string& text)
   {
      std::vector<std::string>& vec = mCenterText[line];
      return std::find(vec.begin(), vec.end(), text) != vec.end();
   }

   /** Add another message to the status panel. */
   void addStatusMessage(const std::string& msg);

   /** Set the panel to dirty.  Next update will rebuild. */
   void setDirty();

   /** Update the status panel if it is needed. */
   void update();

public:  // Configuration params //

   void setWidthHeight(const float w, const float h);

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

   std::string    mHeaderTitle;
   std::string    mCenterTitle;
   std::string    mBottomTitle;

   std::string    mHeaderText;
   std::map<ControlTextLine, std::vector<std::string> > mCenterText;
   std::deque<std::string> mStatusLines;

   float mMetersToAppUnits;

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

   unsigned int mStatusHistorySize; /**< Number of status lines to keep around. */
   float    mStatusTextHeight;     /**< Fixed height (in OpenSG coords) of status text. */


   // Colors/theming
   OSG::Color3f   mBgColor;      /**< Color of the background. */
   float          mBgAlpha;
   OSG::Color3f   mBorderColor;
   OSG::Color3f   mTitleColor;
   OSG::Color3f   mTextColor;

   bool           mDrawDebug;    /**< If true, draw debug geometry. */

protected:
   OSG::ClipPlaneChunkPtr mClipRight;
   OSG::ClipPlaneChunkPtr mClipBottom;
};

}

#endif

