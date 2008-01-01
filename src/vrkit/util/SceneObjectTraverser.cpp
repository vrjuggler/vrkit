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

#include <vrkit/SceneObject.h>
#include <vrkit/util/SceneObjectTraverser.h>


namespace vrkit
{

namespace util
{

SceneObjectTraverser::Result
SceneObjectTraverser::traverse(SceneObjectPtr root,
                               SceneObjectTraverser::enter_func_t enterFunc,
                               SceneObjectTraverser::exit_func_t exitFunc)
{
   Result result = enterFunc(root);

   switch ( result )
   {
      case Skip:
         result = Continue;
         break;
      case Continue:
         result = traverse(root->getChildren(), enterFunc, exitFunc);
         break;
      default:
         break;
   }

   result = exitFunc(root, result);

   return result;
}

SceneObjectTraverser::Result SceneObjectTraverser::
traverse(const std::vector<SceneObjectPtr>& roots,
         SceneObjectTraverser::enter_func_t enterFunc,
         SceneObjectTraverser::exit_func_t exitFunc)
{
   Result result(Continue);

   typedef std::vector<SceneObjectPtr>::const_iterator iter_type;
   for ( iter_type r = roots.begin(); r != roots.end(); ++r )
   {
      result = traverse(*r, enterFunc, exitFunc);
      if ( result == Stop )
      {
         break;
      }
   }

   return result;
}

}

}
