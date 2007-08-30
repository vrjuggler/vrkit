// vrkit is (C) Copyright 2005-2007
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

#ifndef _VRKIT_MOVE_STRATEGY_H_
#define _VRKIT_MOVE_STRATEGY_H_

#include <vrkit/Config.h>

#include <vector>

#include <gmtl/Matrix.h>
#include <gmtl/Point.h>
#include <vpr/DynLoad/Library.h>

#include <vrkit/AbstractPlugin.h>
#include <vrkit/SceneObjectPtr.h>
#include <vrkit/ViewerPtr.h>

#include <vrkit/move/StrategyPtr.h>

#define VRKIT_MOVE_STRATEGY_PLUGIN_API_MAJOR 2
#define VRKIT_MOVE_STRATEGY_PLUGIN_API_MINOR 1


namespace vrkit
{

namespace move
{

/**
 * A pluggable strategy for moving grabbed scene objects.
 *
 * @note This class was renamed from vrkit::MoveStrategy in version 0.47.
 *
 * @since 0.16.0
 */
class VRKIT_CLASS_API Strategy : public AbstractPlugin
{
protected:
   /**
    * Constructor. This does nothing.
    */
   Strategy(const plugin::Info& info);

public:
   /**
    * @since 0.36
    */
   typedef boost::shared_ptr<Strategy> ptr_type;

   /**
    * Returns the name of the entry point function used for retrieving the
    * creator of instances of this plug-in type.
    */
   static std::string getCreatorFuncName()
   {
      return "getMoveStrategyCreator";
   }

   /**
    * Destructor. This does nothing.
    */
   virtual ~Strategy();

   /**
    * Validates that the given dynamic code entity has the query interface
    * needed to get an instance of vrkit::move::Strategy.
    *
    * @pre \p pluginLib must have been loaded successfully.
    *
    * @param pluginLib The code entity (shared library, DLL, executable) to
    *                  validate.
    *
    * @return This method always returns true. If the plug-in is not valid,
    *         an exception is thrown.
    *
    * @throw vrkit::PluginInterfaceException
    *           Thrown if \p pluginLib is not a valid dynamic code entity for
    *           a vrkit::move::Strategy instance.
    */
   static bool validatePluginLib(vpr::LibraryPtr pluginLib);

   /**
    * Initializes this move strategy.
    *
    * @post This move strategy is initialized and ready to be used.
    *
    * @param viewer The VR Juggler application object.
    *
    * @return This object is returned as a shared pointer.
    */
   virtual StrategyPtr init(ViewerPtr viewer) = 0;

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
   virtual void objectsGrabbed(ViewerPtr viewer,
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
   virtual void objectsReleased(ViewerPtr viewer,
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
    *                  parent (pobj_M_obj). This could be retrieved by calling
    *                  \code obj->getMatrix() \endcode except that the
    *                  composition of multiple movement computations may not
    *                  yet have been applied to \p obj. Therefore, this matrix
    *                  should be used instead.
    *
    * @return The new transformation for \p obj (pobj_M_obj').
    *
    * @note Do not call \code obj->getMatrix() \endcode in the implementation
    *       of this method. Use \p curObjPos instead.
    */
   virtual gmtl::Matrix44f computeMove(ViewerPtr viewer, SceneObjectPtr obj,
                                       const gmtl::Matrix44f& vp_M_wand,
                                       const gmtl::Matrix44f& curObjPos) = 0;
};

}

}


#endif /* _VRKIT_MOVE_STRATEGY_H_ */
