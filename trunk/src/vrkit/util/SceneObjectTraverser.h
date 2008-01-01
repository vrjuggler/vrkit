// vrkit is (C) Copyright 2005-2008
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

#ifndef _VRKIT_UTIL_SCENE_OBJECT_TRAVERSER_H_
#define _VRKIT_UTIL_SCENE_OBJECT_TRAVERSER_H_

#include <vrkit/Config.h>

#include <vector>
#include <boost/function.hpp>

#include <vrkit/SceneObjectPtr.h>


namespace vrkit
{

namespace util
{

/** \class SceneObjectTraverser SceneObjectTraverser.h vrkit/util/SceneObjectTraverser.h
 *
 * Basic traverser functions for vrkit scene objects.
 *
 * @note This class was moved into the vrkit::util namespace in version 0.47.
 *
 * @since 0.25.1
 */
class VRKIT_CLASS_API SceneObjectTraverser
{
public:
   /** Traversal results. */
   enum Result
   {
      Continue,         /**< Continue traversing */
      Skip,             /**< Skip the current subtree of scene objects */
      Stop              /**< Stop traversing altogether */
   };

   typedef boost::function<Result (SceneObjectPtr)> enter_func_t;
   typedef boost::function<Result (SceneObjectPtr, Result)> exit_func_t;

   /** @name Traversal Functions */
   //@{
   /**
    * Traverses the tree of vrkit scene objects rooted at \p root using a
    * depth-first search. During traversal, \p enterFunc is invoked when the
    * traverser first encounters a scene object, and \p exitFunc is invoked
    * when exiting the same scene object.
    *
    * @param root      The starting point for the traverser.
    * @param enterFunc The function executed when the traverser enters a new
    *                  scene object.
    * @param exitFunc  The function executed when the traverser has finished
    *                  each scene object subtree. This parameter is optional,
    *                  and it defaults to a dummy exit function if it is not
    *                  specified.
    *
    * @return The result of executing \p exitFunc on the last exited node is
    *         returned to the caller.
    */
   static Result traverse(SceneObjectPtr root, enter_func_t enterFunc,
                          exit_func_t exitFunc = defaultExitFunc);

   /**
    * Traverses the trees of vrkit scene objects rooted at \p roots using a
    * depth-first search on each tree. During traversal, \p enterFunc is
    * invoked when the traverser first encounters a scene object, and
    * \p exitFunc is invoked when exiting the same scene object.
    *
    * @param roots     The collection of starting points for the traverser.
    * @param enterFunc The function executed when the traverser enters a new
    *                  scene object.
    * @param exitFunc  The function executed when the traverser has finished
    *                  each scene object subtree. This parameter is optional,
    *                  and it defaults to a dummy exit function if it is not
    *                  specified.
    *
    * @return The result of executing \p exitFunc on the last exited node is
    *         returned to the caller.
    */
   static Result traverse(const std::vector<SceneObjectPtr>& roots,
                          enter_func_t enterFunc,
                          exit_func_t exitFunc = defaultExitFunc);
   //@}

private:
   /** Dummy trraversal exit function that simply returns \p r. */
   static Result defaultExitFunc(SceneObjectPtr, Result r)
   {
      return r;
   }
};

}

}


#endif /* _VRKIT_UTIL_SCENE_OBJECT_TRAVERSER_H_ */
