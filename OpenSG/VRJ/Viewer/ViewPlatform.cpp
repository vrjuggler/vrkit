#include <OpenSG/VRJ/Viewer/ViewPlatform.h>
#include <OpenSG/VRJ/Viewer/Scene.h>
#include <OpenSG/VRJ/Viewer/Viewer.h>

namespace inf
{

void ViewPlatform::update(ViewerPtr viewer)
{
   mNavStrategy->update(viewer, *this);

   // Update the scene graph transformation here
   CoredTransformPtr xform_node = viewer->getSceneObj()->getTransformRoot();
   OSG::Matrix new_xform;
   gmtl::set(new_xform, getCurPosInv());        // vp_M_vw

   // Set the new transformation on the scene graph
   OSG::beginEditCP(xform_node);
      xform_node->setMatrix(new_xform);
   OSG::endEditCP(xform_node);
}

}

