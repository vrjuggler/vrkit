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

#include <iostream>

#include <OpenSG/OSGGeometry.h>
#include <OpenSG/OSGTransform.h>
#include <OpenSG/OSGTypedFunctors.h>

#include <IOV/Util/OpenSGHelpers.h>
#include <IOV/Util/RemoteAspectFilter.h>


namespace inf
{

RemoteAspectFilter::RemoteAspectFilter()
   : mRemoteAspect(NULL)
   , mNodes(0)
   , mTransforms(0)
   , mGeometries(0)
   , mMaterials(0)
{
   /* Do nothing. */
}

RemoteAspectFilter::~RemoteAspectFilter()
{
   mChangedCallbacks.clear();
   mCreatedCallbacks.clear();
   mDestroyedCallbacks.clear();
}

RemoteAspectFilterPtr RemoteAspectFilter::init(OSG::RemoteAspect* remoteAspect)
{
   // XXX: Do we really need to keep a pointer to this?
   mRemoteAspect = remoteAspect;

   // Setup the aspect callbacks
   OSG::RemoteAspect::Functor changed =
      OSG::osgTypedMethodFunctor2ObjPtrCPtrRef<
         bool, RemoteAspectFilter, OSG::FieldContainerPtr, OSG::RemoteAspect*
      >(this, &RemoteAspectFilter::changedFunction);

   OSG::RemoteAspect::Functor destroyed =
      OSG::osgTypedMethodFunctor2ObjPtrCPtrRef<
         bool, RemoteAspectFilter, OSG::FieldContainerPtr, OSG::RemoteAspect*
      >(this, &RemoteAspectFilter::destroyedFunction);

   OSG::RemoteAspect::Functor created =
      OSG::osgTypedMethodFunctor2ObjPtrCPtrRef<
         bool, RemoteAspectFilter, OSG::FieldContainerPtr, OSG::RemoteAspect*
      >(this, &RemoteAspectFilter::createdFunction);

   // Register interest for all types
   const OSG::UInt32 type_count(OSG::TypeFactory::the()->getNumTypes());
   for ( OSG::UInt32 i = 0; i < type_count; ++i )
   {
      OSG::FieldContainerType* fct =
         OSG::FieldContainerFactory::the()->findType(i);
      if ( fct )
      {
         remoteAspect->registerChanged(*fct, changed);
         remoteAspect->registerDestroyed(*fct, destroyed);
         remoteAspect->registerCreated(*fct, created);
      }
   }

   return shared_from_this();
}

void RemoteAspectFilter::addChangedCallback(OSG::FieldContainerPtr fcp,
                                            callback_t callback)
{
   mChangedCallbacks.insert(callback_map_t::value_type(fcp, callback));
}

void RemoteAspectFilter::addCreatedCallback(OSG::FieldContainerPtr fcp,
                                            callback_t callback)
{
   mCreatedCallbacks.insert(callback_map_t::value_type(fcp, callback));
}

void RemoteAspectFilter::addDestroyedCallback(OSG::FieldContainerPtr fcp,
                                              callback_t callback)
{
   mDestroyedCallbacks.insert(callback_map_t::value_type(fcp, callback));
}

bool RemoteAspectFilter::createdFunction(OSG::FieldContainerPtr& fcp,
                                         OSG::RemoteAspect*)
{
   if ( OSG::Node::getClassType() == fcp->getType() )
   {
      ++mNodes;
   }
   else if ( OSG::Transform::getClassType() == fcp->getType() )
   {
      ++mTransforms;
   }
   else if ( OSG::Geometry::getClassType() == fcp->getType() )
   {
      ++mGeometries;
   }
   else if ( fcp->getType().isDerivedFrom(OSG::Material::getClassType()) )
   {
      ++mMaterials;
   }

   std::cout << "Created: " << fcp->getType().getName() << " "
             << fcp.getFieldContainerId() << std::endl
             << "   name: " << inf::getName(fcp) << std::endl;

   typedef callback_map_t::const_iterator iter_type;
   typedef std::pair<iter_type, iter_type> range_type;

   range_type range = mCreatedCallbacks.equal_range(fcp);
   for ( iter_type i = range.first; i != range.second; ++i )
   {
      (*i).second(fcp);
   }

   range = mCreatedCallbacks.equal_range(OSG::NullFC);
   for ( iter_type i = range.first; i != range.second; ++i )
   {
      (*i).second(fcp);
   }

   return true;
}

/** \brief Default destroyed functor
 */
bool RemoteAspectFilter::destroyedFunction(OSG::FieldContainerPtr& fcp,
                                           OSG::RemoteAspect*)
{
   if ( OSG::Node::getClassType() == fcp->getType() )
   {
      --mNodes;
   }
   else if ( OSG::Transform::getClassType() == fcp->getType() )
   {
      --mTransforms;
   }
   else if ( OSG::Geometry::getClassType() == fcp->getType() )
   {
      --mGeometries;
   }
   else if ( fcp->getType().isDerivedFrom(OSG::Material::getClassType()) )
   {
      --mMaterials;
   }

   std::cout << "Destroyed: " << fcp->getType().getName() << " "
             << fcp.getFieldContainerId() << std::endl
             << "   name: " << inf::getName(fcp) << std::endl;

   typedef callback_map_t::const_iterator iter_type;
   typedef std::pair<iter_type, iter_type> range_type;

   range_type range = mDestroyedCallbacks.equal_range(fcp);
   for ( iter_type i = range.first; i != range.second; ++i )
   {
      (*i).second(fcp);
   }

   range = mDestroyedCallbacks.equal_range(OSG::NullFC);
   for ( iter_type i = range.first; i != range.second; ++i )
   {
      (*i).second(fcp);
   }

   return true;
}

bool RemoteAspectFilter::changedFunction(OSG::FieldContainerPtr& fcp,
                                         OSG::RemoteAspect*)
{
   std::cout << "Changed: " << fcp->getType().getName() << " "
             << fcp.getFieldContainerId() << std::endl
             << "   name: " << inf::getName(fcp) << std::endl;

   typedef callback_map_t::const_iterator iter_type;
   typedef std::pair<iter_type, iter_type> range_type;

   range_type range = mChangedCallbacks.equal_range(fcp);
   for ( iter_type i = range.first; i != range.second; ++i )
   {
      (*i).second(fcp);
   }

   range = mChangedCallbacks.equal_range(OSG::NullFC);
   for ( iter_type i = range.first; i != range.second; ++i )
   {
      (*i).second(fcp);
   }

   return true;
}

}
