// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_RAY_INTERSECTION_STRATEGY_H_
#define _INF_RAY_INTERSECTION_STRATEGY_H_

#include <string>

#include <boost/enable_shared_from_this.hpp>
#include <boost/signals/connection.hpp>

#include <OpenSG/OSGColor.h>
#include <OpenSG/OSGLine.h>
#include <OpenSG/OSGGeometry.h>
#include <OpenSG/OSGSwitch.h>
#include <OpenSG/OSGGeometry.h>
#include <OpenSG/OSGNode.h>

#include <gmtl/Point.h>

#include <jccl/Config/ConfigElementPtr.h>

#include <IOV/ViewerPtr.h>
#include <IOV/Util/Exceptions.h>
#include <IOV/Grab/IntersectionStrategy.h>
#include <IOV/Util/SceneObjectTraverser.h>


namespace inf
{

class RayIntersectionStrategy
   : public IntersectionStrategy
   , public boost::enable_shared_from_this<RayIntersectionStrategy>
{
protected:
   RayIntersectionStrategy(const inf::plugin::Info& info);

public:
   virtual ~RayIntersectionStrategy();

   static std::string getId()
   {
      return "RayIntersection";
   }

   static IntersectionStrategyPtr create(const inf::plugin::Info& info)
   {
      return IntersectionStrategyPtr(new RayIntersectionStrategy(info));
   }

   virtual inf::IntersectionStrategyPtr init(ViewerPtr viewer);

   virtual void update(ViewerPtr viewer);

   virtual SceneObjectPtr
      findIntersection(ViewerPtr viewer,
                       const std::vector<SceneObjectPtr>& objs,
                       gmtl::Point3f& intersectPoint);

   void setVisible(const bool visible);

   void initGeom();

private:
   static std::string getElementType()
   {
      return "ray_intersection_strategy";
   }

   /** @name Intersection Traversal Methods */
   //@{
   SceneObjectTraverser::Result enterFunc(SceneObjectPtr obj);
   void setHit(float enterVal, SceneObjectPtr obj, const OSG::Pnt3f&  point);
   //@}

   /**
    * Configures this intersection strategy.
    *
    * @pre The type of the given config element matches the identifier
    *      returned by getElementType().
    *
    * @param cfgElt The config element to use for configuring this object.
    *
    * @throw inf::PluginException is thrown if the version of the given
    *        config element is too old.
    */
   void configure(jccl::ConfigElementPtr cfgElt);

   OSG::Line            mSelectionRay;    /**< The ray used for selection. */

   // The points used for visualising the ray and hit object
   OSG::GeoPositions3fPtr    mGeomPts;

   // The visualisation geometry, needed for update.
   OSG::GeometryNodePtr mGeomNode;
   OSG::SwitchNodePtr   mSwitchNode;

   boost::signals::connection mRayIsectConn;	/**< Visibility connection object. */

   /** @name Ray Properties */
   //@{
   float        mRayLength;     /**< The length of the rendered "ray" */
   OSG::Color4f mRayDiffuse;
   OSG::Color4f mRayAmbient;
   OSG::Real32  mRayWidth;
   //@}

   /** @name Intersection Traversal Properties */
   //@{
   float                mMinDist;
   SceneObjectPtr       mIntersectObj;
   OSG::Pnt3f           mIntersectPoint;
   gmtl::Matrix44f      m_vp_M_wand;
   //@}

   bool mTriangleIsect;
};

}


#endif /*_INF_RAY_INTERSECTION_STRATEGY_H_*/
