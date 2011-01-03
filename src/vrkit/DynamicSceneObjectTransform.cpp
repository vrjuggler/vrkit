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

#include <boost/mpl/vector.hpp>

#include <OpenSG/OSGFieldContainerFactory.h>

#include <vrkit/util/CoreTypeSeqPredicate.h>
#include <vrkit/DynamicSceneObjectTransform.h>


namespace vrkit
{

DynamicSceneObjectTransform::~DynamicSceneObjectTransform()
{
   /* Do nothing. */ ;
}

DynamicSceneObjectTransform::DynamicSceneObjectTransform()
   : DynamicSceneObject()
{
   /* Do nothing. */ ;
}

DynamicSceneObjectTransformPtr
DynamicSceneObjectTransform::init(OSG::NodePtr node)
{
   typedef boost::mpl::vector<OSG::Transform> core_types;

   util::CoreTypeSeqPredicate<core_types> pred;
   DynamicSceneObjectPtr myself = DynamicSceneObject::init(node, pred, true);

   return boost::dynamic_pointer_cast<DynamicSceneObjectTransform>(myself);
}

}
