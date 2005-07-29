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
    * Sets the text description for the command identified by \p cmd.
    * This overwrites whatever was previously in the buffer for the
    * identified command.
    *
    * @since 0.10.0
    */
   void setControlText(const int cmd, const std::string& desc)
   {
      std::vector<int> cmds(1, cmd);
      setControlText(cmds, desc);
   }

   /**
    * Sets the text description for the command identified by \p cmds.
    * This overwrites whatever was previously in the buffer for the
    * identified command.
    *
    * @since 0.10.0
    */
   void setControlText(const std::vector<int>& cmds, const std::string& desc);

   /**
    * Adds the given text description ifor the command identified by \p cmd
    * to this panel's control section.
    *
    * @since 0.10.0
    */
   void addControlText(const int cmd, const std::string& desc,
                       const unsigned int priority = 1)
   {
      std::vector<int> cmds(1, cmd);
      addControlText(cmds, desc, priority);
   }

   /**
    * Adds the given text description ifor the command identified by \p cmds
    * to this panel's control section.
    *
    * @since 0.10.0
    */
   void addControlText(const std::vector<int>& cmds, const std::string& desc,
                       const unsigned int priority = 1);

   /**
    * Removes the given text description for the command identified by \cmd
    * from this panel's control section.  If the given text is not associated
    * with the identified command, this has no effect.
    *
    * @since 0.10.0
    */
   void removeControlText(const int cmd, const std::string& desc)
   {
      std::vector<int> cmds(1, cmd);
      removeControlText(cmds, desc);
   }

   /**
    * Removes the given text description for the command identified by \cmds
    * from this panel's control section.  If the given text is not associated
    * with the identified command, this has no effect.
    *
    * @since 0.10.0
    */
   void removeControlText(const std::vector<int>& cmds,
                          const std::string& desc);

   /**
    * Determines if the named line contains the given text.
    *
    * @since 0.10.0
    */
   bool hasControlText(const int cmd, const std::string& desc)
   {
      std::vector<int> cmds(1, cmd);
      return hasControlText(cmds, desc);
   }

   /**
    * Determines if the named line contains the given text.
    *
    * @since 0.10.0
    */
   bool hasControlText(const std::vector<int>& cmds, const std::string& desc)
   {
      bool result(false);

      if ( mCenterText.count(cmds) != 0 )
      {
         std::vector<std::string>& vec = mCenterText[cmds];
         result = std::find(vec.begin(), vec.end(), desc) != vec.end();
      }

      return result;
   }

   /** Add another message to the status panel. */
   void addStatusMessage(const std::string& msg);

   /** Set the panel to dirty.  Next update will rebuild. */
   void setDirty();

   /** Update the status panel if it is needed. */
   void update();

public:  // Configuration params //

   void setWidthHeight(const float w, const float h, const float borderWidth=0.0f);

   /**
    * Sets the size of the buffer for the status history.  This
    * affects the number of status lines displayed.
    *
    * @since 0.7.2
    */
   void setStatusHistorySize(const unsigned int size);

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

   typedef std::map<std::vector<int>, std::vector<std::string> > center_text_t;
   center_text_t mCenterText;
   std::deque<std::string> mStatusLines;

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

