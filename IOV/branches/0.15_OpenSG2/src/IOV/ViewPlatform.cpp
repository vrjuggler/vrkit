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

#include <gmtl/External/OpenSGConvert.h>

#include <IOV/ViewPlatform.h>
#include <IOV/Scene.h>
#include <IOV/Viewer.h>

namespace inf
{

void ViewPlatform::update(ViewerPtr viewer)
{
   // Update the scene graph transformation here
   OSG::TransformNodePtr xform_node = viewer->getSceneObj()->getTransformRoot();
   OSG::Matrix new_xform;
   gmtl::set(new_xform, getCurPosInv());        // vp_M_vw

   // Set the new transformation on the scene graph
   OSG::beginEditCP(xform_node);
      xform_node->setMatrix(new_xform);
   OSG::endEditCP(xform_node);
}

}
