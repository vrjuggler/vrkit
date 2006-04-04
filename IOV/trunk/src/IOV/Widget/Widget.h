// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_WIDGET_H_
#define _INF_WIDGET_H_

#include <IOV/Config.h>

#include <OpenSG/OSGConfig.h>
#include <OpenSG/OSGColor.h>
#include <OpenSG/OSGGeometry.h>
#include <OpenSG/OSGMaterial.h>
#include <OpenSG/OSGNode.h>
#include <OpenSG/OSGTransform.h>

#include <IOV/UiBuilder.h>
#include <IOV/StaticSceneObject.h>
#include <IOV/Widget/WidgetPtr.h>
#include <IOV/Widget/Event.h>

#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/signal.hpp>

namespace inf
{

class IOV_CLASS_API Widget : boost::noncopyable, 
                             public StaticSceneObject
{
protected:
   Widget();

public:
   static WidgetPtr create()
   {
      return WidgetPtr(new Widget());
   }

   virtual ~Widget();

   /** Initialize scene graph, fonts and everything else that is used. */
   virtual void init(const float metersToAppUnits);

   virtual void wandEntered();
   virtual void wandExited();
   virtual void wandPressed();
   virtual void wandReleased();
   virtual void wandMoved();

   OSG::NodeRefPtr getRoot()
   { return OSG::NodeRefPtr(mRootWidgetNode.node()); }

   /** Set the panel to dirty.  Next update will rebuild. */
   void setDirty();

   /** Update the status panel if it is needed. */
   void update();

public:  // Configuration params //

   virtual void setWidthHeight(const float w, const float h,
                               const float borderWidth = 0.0f);

   void move(const OSG::Pnt3f& pnt);

   /**
    * Positions this widget using the given transformation.
    *
    * @post The transform core of \c mRootWidgetNode has its matrix replaced
    *       by \p xform.
    *
    * @param xform The transformation to use for positioning this widget.
    *
    * @since 0.19.2
    */
   void move(const OSG::Matrix& xform);

   void setMaterial(OSG::MaterialPtr mat = OSG::MaterialPtr());
protected:
   virtual void updatePanelScene();

protected:
   inf::UiBuilder        mBuilder;
   bool                  mIsDirty;     /**< When true, we need a rebuild of the panel. */

   OSG::TransformNodePtr mRootWidgetNode;
   OSG::GeometryNodePtr  mWidgetGeomNode;

   float mMetersToAppUnits;

   /** Panel sizes are in OpenSG units.
    * Everything else is in normalized percentages or discrete num of's (ie. num lines).
    * Thus all real sizes are derived based on the widget size set.
    * This "should" make it possible to resize the widget and get good results.
    */
   float    mWidth, mHeight;  /**< Panel size in OpenSG units. */

   // Colors/theming
   OSG::Color3f   mBgColor;      /**< Color of the background. */
   float          mBgAlpha;

   bool           mDrawDebug;    /**< If true, draw debug geometry. */
};

}

#endif

