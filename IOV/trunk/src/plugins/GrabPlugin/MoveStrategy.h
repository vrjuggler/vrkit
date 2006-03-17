// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_MOVE_STRATEGY_H_
#define _INF_MOVE_STRATEGY_H_

#include <boost/noncopyable.hpp>
#include <OpenSG/OSGTransform.h>
#include <gmtl/Matrix.h>
#include <vpr/DynLoad/Library.h>
#include <IOV/ViewerPtr.h>

#include "MoveStrategyPtr.h"

#define INF_MOVE_STRATEGY_PLUGIN_API_MAJOR 1
#define INF_MOVE_STRATEGY_PLUGIN_API_MINOR 0


namespace inf
{

class MoveStrategy : public boost::noncopyable
{
public:
   /**
    * Returns the name of the entry point function used for retrieving the
    * creator of instances of this plug-in type.
    */
   static std::string getCreatorFuncName()
   {
      return "getMoveStrategyCreator";
   }

   /**
    * Validates that the given dynamic code entity has the query interface
    * needed to get an instance of inf::MoveStrategy.
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
    *        valid dynamic code entity for an inf::MoveStrategy instance.
    */
   static bool validatePluginLib(vpr::LibraryPtr pluginLib);

   virtual ~MoveStrategy();

   virtual void init(ViewerPtr viewer) = 0;

   virtual void objectGrabbed(inf::ViewerPtr viewer,
                              OSG::TransformNodePtr obj,
                              const gmtl::Matrix44f& vp_M_wand) = 0;

   virtual void objectReleased(inf::ViewerPtr viewer,
                               OSG::TransformNodePtr obj) = 0;

   virtual gmtl::Matrix44f computeMove(inf::ViewerPtr viewer,
                                       OSG::TransformNodePtr obj,
                                       const gmtl::Matrix44f& vp_M_wand,
                                       gmtl::Matrix44f& curObjPos) = 0;

protected:
   MoveStrategy();
};

}

#endif /*_INF_MOVE_STRATEGY_H_*/
