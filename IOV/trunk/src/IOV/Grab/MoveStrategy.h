// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_MOVE_STRATEGY_H_
#define _INF_MOVE_STRATEGY_H_

#include <IOV/Config.h>

#include <boost/noncopyable.hpp>
#include <OpenSG/OSGTransform.h>
#include <gmtl/Matrix.h>
#include <vpr/DynLoad/Library.h>
#include <IOV/ViewerPtr.h>

#include <IOV/Grab/MoveStrategyPtr.h>

#define INF_MOVE_STRATEGY_PLUGIN_API_MAJOR 1
#define INF_MOVE_STRATEGY_PLUGIN_API_MINOR 0


namespace inf
{

/**
 * A pluggable strategy for moving a grabbed scene object.
 */
class IOV_CLASS_API MoveStrategy : public boost::noncopyable
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

   /**
    * Destructor. This does nothing.
    */
   virtual ~MoveStrategy();

   /**
    * Initializes this move strategy.
    *
    * @post This move strategy is initialized and ready to be used.
    *
    * @param viewer The VR Juggler application object.
    */
   virtual void init(ViewerPtr viewer) = 0;

   /**
    * Indicates to this move strategy that an object has been grabbed.
    *
    * @pre This move strategy has been initialized. No other object is
    *      currently grabbed.
    *
    * @param viewer         The VR Juggler application object.
    * @param obj            The node in the scene graph that has been grabbed.
    * @param intersectPoint The previously determined point of intersection.
    * @param vp_M_wand      The wand transformation in view platform
    *                       coordiantes.
    */
   virtual void objectGrabbed(inf::ViewerPtr viewer,
                              OSG::TransformNodePtr obj,
                              gmtl::Point3f& intersectPoint,
                              const gmtl::Matrix44f& vp_M_wand) = 0;

   /**
    * Indicates to this move strategy that the grabbed object has been
    * released.
    *
    * @pre objectGrabbed() was called, and \p obj was the object passed to it.
    *
    * @param viewer The VR Juggler application object.
    * @param obj    The grabbed node in the scene graph that is now released.
    */
   virtual void objectReleased(inf::ViewerPtr viewer,
                               OSG::TransformNodePtr obj) = 0;

   /**
    * Computes the new transformation for \p obj based on the behavior of this
    * move strategy. The results of multiple movement strategies may be
    * composed to compute the new transformation for \p obj.
    *
    * @pre objectGrabbed() was called, and \p obj was the object passed to it.
    *      Hence, an object is grabbed.
    *
    * @param viewer    The VR Juggler application object.
    * @param obj       The node in the scene graph that is grabbed.
    * @param vp_M_wand The wand transformation in view platform coordiantes.
    * @param curObjPos The current transformation of \p obj relative to its
    *                  parent. This could be retrieved by calling
    *                  \code obj->getMatrix() \endcode except that the
    *                  composition of multiple movement computations may not
    *                  yet have been applied to \p obj. Therefore, this matrix
    *                  should be used instead.
    *
    * @return The new transformation for \p obj.
    *
    * @note Do not call \code obj->getMatrix() \endcode in the implementation
    *       of this method. Use \p curObjPos instead.
    */
   virtual gmtl::Matrix44f computeMove(inf::ViewerPtr viewer,
                                       OSG::TransformNodePtr obj,
                                       const gmtl::Matrix44f& vp_M_wand,
                                       gmtl::Matrix44f& curObjPos) = 0;

protected:
   /**
    * Constructor. This does nothing.
    */
   MoveStrategy();
};

}


#endif /*_INF_MOVE_STRATEGY_H_*/
