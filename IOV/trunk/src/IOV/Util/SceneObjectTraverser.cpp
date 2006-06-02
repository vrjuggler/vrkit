#include <IOV/SceneObject.h>
#include <IOV/Util/SceneObjectTraverser.h>


namespace inf
{

SceneObjectTraverser::Result
SceneObjectTraverser::traverse(inf::SceneObjectPtr root,
                               SceneObjectTraverser::enter_func_t enterFunc,
                               SceneObjectTraverser::exit_func_t exitFunc)
{
   Result result = enterFunc(root);

   switch ( result )
   {
      case Skip:
         result = Continue;
      case Continue:
         result = traverse(root->getChildren(), enterFunc, exitFunc);
      default:
         break;
   }

   result = exitFunc(root, result);

   return result;
}

SceneObjectTraverser::Result SceneObjectTraverser::
traverse(const std::vector<inf::SceneObjectPtr>& roots,
         SceneObjectTraverser::enter_func_t enterFunc,
         SceneObjectTraverser::exit_func_t exitFunc)
{
   Result result(Continue);

   std::vector<inf::SceneObjectPtr>::const_iterator r;
   for ( r = roots.begin(); r != roots.end(); ++r )
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
