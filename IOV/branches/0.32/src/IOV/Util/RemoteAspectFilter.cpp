// Copyright (C) Infiscape Corporation 2005-2007

#include <iostream>
#include <boost/bind.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/operations.hpp>

#include <OpenSG/OSGBlendChunk.h>

#include <vpr/vpr.h>
#include <vpr/System.h>
#include <vpr/Util/FileUtils.h>
#include <jccl/Config/ConfigElement.h>

#include <IOV/EventData.h>
#include <IOV/SceneObject.h>
#include <IOV/Viewer.h>
#include <IOV/Util/Exceptions.h>
#include <IOV/Util/OpenSGHelpers.h>

#include <IOV/Util/RemoteAspectFilter.h>


namespace fs = boost::filesystem;

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
   OSG::RemoteAspect::Functor changed = OSG::osgTypedMethodFunctor2ObjPtrCPtrRef < bool,
       RemoteAspectFilter, OSG::FieldContainerPtr, OSG::RemoteAspect*> (this, &RemoteAspectFilter::changedFunction);

   OSG::RemoteAspect::Functor destroyed = OSG::osgTypedMethodFunctor2ObjPtrCPtrRef < bool,
       RemoteAspectFilter, OSG::FieldContainerPtr, OSG::RemoteAspect*> (this, &RemoteAspectFilter::destroyedFunction);

   OSG::RemoteAspect::Functor created = OSG::osgTypedMethodFunctor2ObjPtrCPtrRef < bool,
       RemoteAspectFilter, OSG::FieldContainerPtr, OSG::RemoteAspect*> (this, &RemoteAspectFilter::createdFunction);

   // Register interest for all types
   for(OSG::UInt32 i = 0; i < OSG::TypeFactory::the()->getNumTypes(); ++i)
   {
       OSG::FieldContainerType* fct = OSG::FieldContainerFactory::the()->findType(i);
       if(fct)
       {
          remoteAspect->registerChanged(*fct, changed);
          remoteAspect->registerDestroyed(*fct, destroyed);
          remoteAspect->registerCreated(*fct, created);
       }
   }

   return shared_from_this();
}

void RemoteAspectFilter::addChangedCallback(OSG::FieldContainerPtr fcp, boost::function< void (OSG::FieldContainerPtr)> callback)
{
   mChangedCallbacks.insert(callback_map_t::value_type(fcp, callback));
}

void RemoteAspectFilter::addCreatedCallback(OSG::FieldContainerPtr fcp, boost::function< void (OSG::FieldContainerPtr)> callback)
{
   mCreatedCallbacks.insert(callback_map_t::value_type(fcp, callback));
}

void RemoteAspectFilter::addDestroyedCallback(OSG::FieldContainerPtr fcp, boost::function< void (OSG::FieldContainerPtr)> callback)
{
   mDestroyedCallbacks.insert(callback_map_t::value_type(fcp, callback));
}

/* */
bool RemoteAspectFilter::createdFunction(OSG::FieldContainerPtr &fcp, OSG::RemoteAspect *)
{
   if(OSG::Node::getClassType() == fcp->getType())
   { mNodes++; }
   if(OSG::Transform::getClassType() == fcp->getType())
   { mTransforms++; }
   if(OSG::Geometry::getClassType() == fcp->getType())
   { mGeometries++; }
   if (fcp->getType().isDerivedFrom(OSG::Material::getClassType()))
   { mMaterials++; }

   std::cout << "Created: " << fcp->getType().getName().str() << " " << fcp.getFieldContainerId() << std::endl;
   std::cout << "   name: " << inf::getName(fcp) << std::endl;

   std::pair<callback_map_t::const_iterator, callback_map_t::const_iterator> range = mCreatedCallbacks.equal_range(fcp);
   for (callback_map_t::const_iterator i = range.first; i != range.second; ++i)
   {
      (*i).second(fcp);
   }

   range = mCreatedCallbacks.equal_range(OSG::NullFC);
   for (callback_map_t::const_iterator i = range.first; i != range.second; ++i)
   {
      (*i).second(fcp);
   }

   return true;
}

/** \brief Default destroyed functor
 */
bool RemoteAspectFilter::destroyedFunction(OSG::FieldContainerPtr &fcp, OSG::RemoteAspect *)
{
   if(OSG::Node::getClassType() == fcp->getType())
   { mNodes--; }
   if(OSG::Transform::getClassType() == fcp->getType())
   { mTransforms--; }
   if(OSG::Geometry::getClassType() == fcp->getType())
   { mGeometries--; }
   if (fcp->getType().isDerivedFrom(OSG::Material::getClassType()))
   { mMaterials--; }
   std::cout << "Destroyed: " << fcp->getType().getName().str() << " " << fcp.getFieldContainerId() << std::endl;
   std::cout << "   name: " << inf::getName(fcp) << std::endl;

   std::pair<callback_map_t::const_iterator, callback_map_t::const_iterator> range = mDestroyedCallbacks.equal_range(fcp);
   for (callback_map_t::const_iterator i = range.first; i != range.second; ++i)
   {
      (*i).second(fcp);
   }

   range = mDestroyedCallbacks.equal_range(OSG::NullFC);
   for (callback_map_t::const_iterator i = range.first; i != range.second; ++i)
   {
      (*i).second(fcp);
   }


   return true;
}

bool RemoteAspectFilter::changedFunction(OSG::FieldContainerPtr &fcp, OSG::RemoteAspect *)
{
   std::cout << "Changed: " << fcp->getType().getName().str() << " " << fcp.getFieldContainerId() << std::endl;
   std::cout << "   name: " << inf::getName(fcp) << std::endl;

   std::pair<callback_map_t::const_iterator, callback_map_t::const_iterator> range = mChangedCallbacks.equal_range(fcp);
   for (callback_map_t::const_iterator i = range.first; i != range.second; ++i)
   {
      (*i).second(fcp);
   }

   range = mChangedCallbacks.equal_range(OSG::NullFC);
   for (callback_map_t::const_iterator i = range.first; i != range.second; ++i)
   {
      (*i).second(fcp);
   }

   return true;
}
}
