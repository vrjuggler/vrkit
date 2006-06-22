// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_MOVE_STRATEGY_H_
#define _INF_MOVE_STRATEGY_H_

#include <IOV/Config.h>

#include <vector>
#include <boost/noncopyable.hpp>

#include <gmtl/Matrix.h>
#include <gmtl/Point.h>
#include <vpr/DynLoad/Library.h>

#include <IOV/SceneObjectPtr.h>
#include <IOV/ViewerPtr.h>

#include <IOV/Grab/MoveStrategyPtr.h>

#define INF_MOVE_STRATEGY_PLUGIN_API_MAJOR 1
#define INF_MOVE_STRATEGY_PLUGIN_API_MINOR 2


namespace inf
{

/**
 * A pluggable strategy for moving grabbed scene objects.
 *
 * @since 0.16.0
 */
class IOV_CLASS_API MoveStrategy : boost::noncopyable
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
    *
    * @return This object is returned as a shared pointer.
    */
   virtual MoveStrategyPtr init(ViewerPtr viewer) = 0;

   /**
    * Indicates to this move strategy that one or more objects have been
    * grabbed. This method is invoked at most once per frame.
    *
    * @pre This move strategy has been initialized.
    *
    * @param viewer         The VR Juggler application object.
    * @param objs           The nodes in the scene graph that have been
    *                       grabbed.
    * @param intersectPoint The previously determined point of intersection.
    * @param vp_M_wand      The wand transformation in view platform
    *                       coordiantes.
    *
    * @since 0.29.0
    */
   virtual void objectsGrabbed(inf::ViewerPtr viewer,
                               const std::vector<SceneObjectPtr>& objs,
                               const gmtl::Point3f& intersectPoint,
                               const gmtl::Matrix44f& vp_M_wand) = 0;

   /**
    * Indicates to this move strategy that the grabbed object has been
    * released. This method is invoked at most once per frame.
    *
    * @pre objectGrabbed() was called, and \p objs contains one or more of the
    *      objects passed to it. It may not contain all of the currently
    *      grabbed scene objects if only a subset have been released.
    *
    * @param viewer The VR Juggler application object.
    * @param objs   The grabbed nodes in the scene graph that are now released.
    *
    * @since 0.29.0
    */
   virtual void objectsReleased(inf::ViewerPtr viewer,
                                const std::vector<SceneObjectPtr>& objs) = 0;

   /**
    * Computes the new transformation for \p obj based on the behavior of this
    * move strategy. The results of multiple movement strategies may be
    * composed to compute the new transformation for \p obj. This method is
    * invoked iteratively for each of the scene objects that are grabbed.
    *
    * @pre objectGrabbed() was called, and \p obj was one of the objects
    *      passed to it. Hence, the given scene object is grabbed.
    *
    * @param viewer    The VR Juggler application object.
    * @param obj       A scene object that is currently grabbed.
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
                                       SceneObjectPtr obj,
                                       const gmtl::Matrix44f& vp_M_wand,
                                       const gmtl::Matrix44f& curObjPos) = 0;


#if defined(WIN32) || defined(WIN64)
   /**
    * Overlaod delete so that we can delete our memory correctly. This is
    * necessary for DLLs on Windows to release memory from the correct memory
    * space. All subclasses must overload delete similarly.
    */
   void operator delete(void* p)
   {
      if ( NULL != p )
      {
         GrabStrategy* strategy_ptr = static_cast<GrabStrategy*>(p);
         strategy_ptr->destroy();
      }
   }
#endif

protected:
   /**
    * Subclasses must implement this so that dynamically loaded plug-ins
    * delete themselves in the correct memory space. This uses the Template
    * Method Pattern.
    */
   virtual void destroy() = 0;

   /**
    * Constructor. This does nothing.
    */
   MoveStrategy();
};

}


#endif /*_INF_MOVE_STRATEGY_H_*/
