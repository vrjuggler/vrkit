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

#ifndef _VRKIT_WIDGET_H_
#define _VRKIT_WIDGET_H_

#include <vrkit/Config.h>

#include <boost/noncopyable.hpp>
#include <boost/signal.hpp>

#include <OpenSG/OSGConfig.h>
#include <OpenSG/OSGColor.h>
#include <OpenSG/OSGGeometry.h>
#include <OpenSG/OSGMaterial.h>
#include <OpenSG/OSGNode.h>
#include <OpenSG/OSGTransform.h>

#include <vrkit/UiBuilder.h>
#include <vrkit/StaticSceneObject.h>
#include <vrkit/WidgetPtr.h>


namespace vrkit
{

/** \class Widget Widget.h vrkit/Widget.h
 *
 * Base class for all 3D user interface widgets.
 */
class VRKIT_CLASS_API Widget
   : private boost::noncopyable
   , public StaticSceneObject
{
protected:
   Widget();

public:
   static WidgetPtr create()
   {
      return WidgetPtr(new Widget());
   }

   virtual ~Widget();

   /**
    * Initialize scene graph, fonts and everything else that is used.
    *
    * @param metersToAppUnits The conversion factor from meters (VR Juggler's
    *                         internal units) to the application-specific
    *                         units.
    *
    * @return This object is returned as a shared pointer.
    */
   virtual WidgetPtr init(const float metersToAppUnits);

   virtual void wandEntered();
   virtual void wandExited();
   virtual void wandPressed();
   virtual void wandReleased();
   virtual void wandMoved();

   OSG::NodeRefPtr getRoot()
   {
      return OSG::NodeRefPtr(mRootWidgetNode.node());
   }

   /** Set the panel to dirty.  Next update will rebuild. */
   void setDirty();

   /** Update the status panel if it is needed. */
   void update();

public:  // Configuration params //

   virtual void setWidthHeight(const float w, const float h,
                               const float borderWidth = 0.0f);

   void move(const OSG::Pnt3f& pnt);

   /**
    * Positions this widget using the given transformation. This overrides
    * vrkit::SceneObject::moveTo().
    *
    * @post The transform core of \c mRootWidgetNode has its matrix replaced
    *       by \p xform.
    *
    * @param xform The transformation to use for positioning this widget.
    */
   void moveTo(const OSG::Matrix& xform);

   void setMaterial(OSG::MaterialPtr mat = OSG::MaterialPtr());

protected:
   virtual void updatePanelScene();

   UiBuilder mBuilder;
   bool      mIsDirty;     /**< When true, we need a rebuild of the panel. */

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


#endif /* _VRKIT_WIDGET_H_ */
