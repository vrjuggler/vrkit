// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_WIDGET_SPHERE_BUTTON_H_
#define _INF_WIDGET_SPHERE_BUTTON_H_

#include <IOV/Config.h>

#include <IOV/Widget/Widget.h>
#include <IOV/Widget/SphereButtonPtr.h>
#include <IOV/GeometryHighlightTraverser.h>
#include <boost/signal.hpp>

namespace inf
{

class IOV_CLASS_API SphereButton : public Widget
{
protected:
   SphereButton();

public:
   virtual ~SphereButton();
   static SphereButtonPtr create()
   {
      return SphereButtonPtr(new SphereButton());
   }

   virtual void init(const float metersToAppUnits);

   virtual void wandEntered();
   virtual void wandExited();
   virtual void wandPressed();
   virtual void wandReleased();
   virtual void wandMoved();

   // SIGNALS
   boost::signal< int (), Event::ResultOperator > mClickedSignal;
protected:
   GeometryHighlightTraverser mGeomTraverser;
   unsigned int mHoverHighlightID;
   unsigned int mPressedHighlightID;
   unsigned int mActiveHighlightID;
   OSG::Color3f mIntersectColor;
   OSG::Color3f mGrabColor;
   virtual void updatePanelScene();
   bool mPressed;
};

}

#endif

