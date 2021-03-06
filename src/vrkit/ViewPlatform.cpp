// vrkit is (C) Copyright 2005-2011
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

#include <gmtl/External/OpenSGConvert.h>

#include <vrkit/Scene.h>
#include <vrkit/Viewer.h>
#include <vrkit/ViewPlatform.h>


namespace vrkit
{

ViewPlatform::ViewPlatform()
{
   /* Do nothing. */ ;
}

void ViewPlatform::update(ViewerPtr viewer)
{
   // Update the scene graph transformation here
   OSG::TransformNodePtr xform_node = viewer->getSceneObj()->getTransformRoot();
   OSG::Matrix new_xform;
   gmtl::set(new_xform, getCurPosInv());        // vp_M_vw

   // Set the new transformation on the scene graph.
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor xnce(xform_node.core(), OSG::Transform::MatrixFieldMask);
#endif
   xform_node->setMatrix(new_xform);
}

}
