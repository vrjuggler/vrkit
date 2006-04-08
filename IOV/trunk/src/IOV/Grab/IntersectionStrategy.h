// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_INTERSECTION_STRATEGY_H_
#define _INF_INTERSECTION_STRATEGY_H_

#include <IOV/Config.h>

#include <boost/noncopyable.hpp>
#include <OpenSG/OSGTransform.h>
#include <vpr/DynLoad/Library.h>
#include <gmtl/Point.h>

#include <IOV/SceneObjectPtr.h>
#include <IOV/ViewerPtr.h>

#include <IOV/Grab/IntersectionStrategyPtr.h>

#include <vector>

#define INF_ISECT_STRATEGY_PLUGIN_API_MAJOR 1
#define INF_ISECT_STRATEGY_PLUGIN_API_MINOR 0


namespace inf
{

/**
 * A pluggable strategy for detecting intersection with a scene object.
 *
 * @since 0.16.0
 */
class IOV_CLASS_API IntersectionStrategy : boost::noncopyable
{
public:
   /**
    * Returns the name of the entry point function used for retrieving the
    * creator of instances of this plug-in type.
    */
   static std::string getCreatorFuncName()
   {
      return "getIntersectionStrategyCreator";
   }

   /**
    * Validates that the given dynamic code entity has the query interface
    * needed to get an instance of inf::IntersectionStrategy.
    *
    * @pre \p pluginLib must have been loaded successfully.
    *
    * @param pluginLib The code entity (shared library, DLL, executable) to
    *                  validate.
    *
    * @return This method always returns true. If the plug-in is not valid,
    *         an exception is thrown.
    *
    * @throw inf::PluginInterfaceException is thrown if \p pluginLib is not a
    *        valid dynamic code entity for an inf::IntersectionStrategy
    *        instance.
    */
   static bool validatePluginLib(vpr::LibraryPtr pluginLib);

   /**
    * Destructor. This does nothing.
    */
   virtual ~IntersectionStrategy();

   /**
    * Initializes this intersection strategy.
    *
    * @post This intersection strategy is initialized and ready to be used.
    *
    * @param viewer The VR Juggler application object.
    *
    * @return This object is returned as a shared pointer.
    */
   virtual IntersectionStrategyPtr init(ViewerPtr viewer) = 0;

   /**
    * Updates this intersection strategy. This method is invoked once per
    * frame to give the intersection strategy an opportunity to update its
    * state. For exmaple, an intersection strategy may be rendering something
    * in the scene to assist with the intersection operation, and overriding
    * this method is the place to perform updates to that rendered
    * information.
    *
    * @pre This intersection strategy has been initialized.
    *
    * @param viewer The VR Juggler application object.
    */
   virtual void update(ViewerPtr)
   {
      /* Do nothing. */ ;
   }

   /**
    * Attempts to find an intersection with a scene object. This is where the
    * real work is done. If a point of intersection is found, the intersected
    * scene object is returned as is the point of intersection.
    *
    * @pre This intersection strategy has been initialized.
    *
    * @param viewer         The VR Juggler application object.
    * @param intersectPoint Storage for the point of intersection if an
    *                       intersection is detected.
    *
    * @return The intersected scene object is returned or \c OSG::NullFC if no
    *         intersection was found.
    *
    * @note The value of \p intersectPoint is undefined except when the
    *       return value of this method is not \c OSG::NullFC.
    */
   virtual SceneObjectPtr
      findIntersection(ViewerPtr viewer, const std::vector<SceneObjectPtr>& objs,
                       gmtl::Point3f& intersectPoint) = 0;

protected:
   /**
    * Constructor. This does nothing.
    */
   IntersectionStrategy();
};

}


#endif /*_INF_INTERSECTION_STRATEGY_H_*/
