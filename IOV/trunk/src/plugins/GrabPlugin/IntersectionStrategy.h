// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_INTERSECTION_STRATEGY_H_
#define _INF_INTERSECTION_STRATEGY_H_

#include <boost/noncopyable.hpp>
#include <OpenSG/OSGTransform.h>
#include <vpr/DynLoad/Library.h>
#include <IOV/ViewerPtr.h>

#include "IntersectionStrategyPtr.h"

#define INF_ISECT_STRATEGY_PLUGIN_API_MAJOR 1
#define INF_ISECT_STRATEGY_PLUGIN_API_MINOR 0


namespace inf
{

class IntersectionStrategy : public boost::noncopyable
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

   virtual ~IntersectionStrategy();

   virtual void init(ViewerPtr viewer) = 0;

   virtual void update(ViewerPtr)
   {;}

   virtual OSG::TransformNodePtr findIntersection(ViewerPtr viewer) = 0;

protected:
   IntersectionStrategy();
};

}
#endif /*_INF_INTERSECTION_STRATEGY_H_*/
