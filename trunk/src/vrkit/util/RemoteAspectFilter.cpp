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
#include <boost/bind.hpp>

#include <OpenSG/OSGGeometry.h>
#include <OpenSG/OSGTransform.h>
#if OSG_MAJOR_VERSION < 2
#  include <OpenSG/OSGTypedFunctors.h>
#endif
#include <OpenSG/OSGTypeFactory.h>
#include <OpenSG/OSGRemoteAspect.h>

#include <vrkit/util/OpenSGHelpers.h>
#include <vrkit/util/RemoteAspectFilter.h>


namespace vrkit
{

namespace util
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
#if OSG_MAJOR_VERSION < 2
      OSG::osgTypedMethodFunctor2ObjPtrCPtrRef<
         bool, RemoteAspectFilter, OSG::FieldContainerPtr, OSG::RemoteAspect*
      >(this, &RemoteAspectFilter::changedFunction);
#else
      boost::bind(&RemoteAspectFilter::changedFunction, this, _1, _2);
#endif

   OSG::RemoteAspect::Functor destroyed =
#if OSG_MAJOR_VERSION < 2
      OSG::osgTypedMethodFunctor2ObjPtrCPtrRef<
         bool, RemoteAspectFilter, OSG::FieldContainerPtr, OSG::RemoteAspect*
      >(this, &RemoteAspectFilter::destroyedFunction);
#else
      boost::bind(&RemoteAspectFilter::destroyedFunction, this, _1, _2);
#endif

   OSG::RemoteAspect::Functor created =
#if OSG_MAJOR_VERSION < 2
      OSG::osgTypedMethodFunctor2ObjPtrCPtrRef<
         bool, RemoteAspectFilter, OSG::FieldContainerPtr, OSG::RemoteAspect*
      >(this, &RemoteAspectFilter::createdFunction);
#else
      boost::bind(&RemoteAspectFilter::createdFunction, this, _1, _2);
#endif

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

bool RemoteAspectFilter::createdFunction(field_container_ptr_type fcp,
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
#if OSG_MAJOR_VERSION < 2
             << fcp.getFieldContainerId()
#else
             << OSG::getContainerId(fcp)
#endif
             << "\n   name: " << vrkit::util::getName(fcp) << std::endl;

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

// Default destroyed functor.
bool RemoteAspectFilter::destroyedFunction(field_container_ptr_type fcp,
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
#if OSG_MAJOR_VERSION < 2
             << fcp.getFieldContainerId()
#else
             << OSG::getContainerId(fcp)
#endif
             << "\n   name: " << vrkit::util::getName(fcp) << std::endl;

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

bool RemoteAspectFilter::changedFunction(field_container_ptr_type fcp,
                                         OSG::RemoteAspect*)
{
   std::cout << "Changed: " << fcp->getType().getName() << " "
#if OSG_MAJOR_VERSION < 2
             << fcp.getFieldContainerId()
#else
             << OSG::getContainerId(fcp)
#endif
             << "\n   name: " << vrkit::util::getName(fcp) << std::endl;

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

}
