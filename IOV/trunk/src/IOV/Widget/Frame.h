// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_WIDGET_FRAME_H_
#define _INF_WIDGET_FRAME_H_

#include <IOV/Config.h>

#include <OpenSG/OSGColor.h>

#include <IOV/Widget/Widget.h>
#include <IOV/Widget/FramePtr.h>

namespace inf
{

class IOV_CLASS_API Frame : public Widget
{
protected:
   Frame()
   {;}

public:  // Configuration params //
   static FramePtr create()
   {
      return FramePtr(new Frame());
   }

   virtual void init(const float metersToAppUnits);

   virtual void setWidthHeight(const float w, const float h, const float borderWidth=0.0f);

protected:
   virtual void updatePanelScene();

protected:
   float    mBorderWidth;           /**< Width of the border in real units. */
   float    mBorderDepth;           /**< Depth of the border in real units. */

   // Colors/theming
   OSG::Color3f   mBorderColor;

   bool           mDrawDebug;    /**< If true, draw debug geometry. */
};

}

#endif

