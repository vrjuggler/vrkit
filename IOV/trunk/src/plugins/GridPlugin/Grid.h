// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_GRID_H_
#define _INF_GRID_H_

#include <boost/enable_shared_from_this.hpp>

#include <OpenSG/OSGColor.h>
#include <OpenSG/OSGNodePtr.h>
#include <OpenSG/OSGTransform.h>
#include <OpenSG/OSGGeometry.h>

#include <gmtl/Point.h>
#include <gmtl/EulerAngle.h>
#include <gmtl/Matrix.h>

#include <jccl/Config/ConfigElementPtr.h>

#include "GridPtr.h"


namespace inf
{

class Grid : public boost::enable_shared_from_this<Grid>
{
public:
   static GridPtr create()
   {
      return GridPtr(new Grid());
   }

   /**
    * Initializes/configures this grid.
    *
    * @post This grid is fully configured and ready to be used.
    *
    * @param cfgElt A config element containing the parameters needed to
    *               initialize this grid fully.
    *
    * @throw inf::PluginException is thrown if \p cfgElt contains bad data,
    *        is out of date, or is otherwise unusable.
    */
   void init(jccl::ConfigElementPtr cfgElt);

   const std::string& getName() const
   {
      return mName;
   }

   OSG::NodeRefPtr getRoot() const;

   /**
    * Indicates whether this grid is visible.
    *
    * @see setVisible()
    */
   bool isVisible();

   /**
    * Changes the visiblity of this grid.
    *
    * @post The OpenSG subtree for this grid has its "active" traversal flag
    *       set to match \p visible.
    *
    * @param visible A flag indicating whether this grid should be visible.
    */
   void setVisible(const bool visible);

   /**
    * Changes the selected state of this grid. When a grid is selected, a
    * transparent plane is rendered in the same color as the grid (if the
    * grid is visible).
    *
    * @post The OpenSG subtree rooted at \c mPlaneNode has its "active"
    *       traversal flag set to match \p selected.
    *
    * @param selected A flag indicating whether this grid is selected.
    */
   void setSelected(const bool selected);

   /**
    * Changes the transformation for this grid to be \p xform.
    *
    * @post The transformation matrix for \c mRoot is set to \p xform.
    *
    * @param xform The new transformation for this grid.
    */
   void move(const OSG::Matrix& xform);

   /**
    * Changes the transformation for this grid to be \p xform. This method is
    * a helper for converting from the given GMTL matrix to an OpenSG matrix.
    *
    * @post The transformation matrix for \c mRoot is set to \p xform.
    *
    * @param xform The new transformation for this grid.
    */
   void move(const gmtl::Matrix44f& xform);

   /**
    * Resets the transformation for this grid back to its initial position.
    */
   void reset();

   /**
    * Retrieves the current transformation for this grid.
    */
   OSG::Matrix getCurrentXform();

protected:
   Grid()
   {
      /* Do nothing. */ ;
   }

private:
   enum Corner
   {
      LOWER_LEFT,       /**< Lower left corner of the grid */
      LOWER_RIGHT,      /**< Lower right corner of the grid */
      UPPER_RIGHT,      /**< Upper right corner of the grid */
      UPPER_LEFT        /**< Upper left corner of the grid */
   };

   /**
    * Initializes the geometry for this grid.
    *
    * @post \c mInitXform is set to a transformation matrix based on
    *       \p cornerPos and \p rot. \c mRoot is initialized and has
    *       \c mGridNode and \c mPlaneNode as its children. \c mGridNode and
    *       \c mPlanenode are initialized. 
    *
    * @param width       The total width of the rendered grid in feet.
    * @param height      The total height of the rendered grid in feet.
    * @param granularity The width and height (in feet) of a cell in the
    *                    rendered grid. Cells are squares.
    * @param corner      The corner represented by \p cornerPos.
    * @param cornerPos   The position of a corner of the grid. The corner used
    *                    is determined by \p corner.
    * @param rot         The rotation of this grid.
    * @param color       The color of this grid and its selection plane.
    */
   void initGeometry(const OSG::Real32 width, const OSG::Real32 height,
                     const OSG::Real32 granularity, const Corner corner,
                     const OSG::Vec3f& cornerPos,
                     const OSG::Quaternion& rot,
                     const OSG::Color3f& color);

   std::string           mName;
   OSG::Matrix           mInitXform;
   OSG::TransformNodePtr mRoot;
   OSG::GeometryNodePtr  mGridNode;
   OSG::GeometryNodePtr  mPlaneNode;
};

}


#endif /* _INF_GRID_H_ */
