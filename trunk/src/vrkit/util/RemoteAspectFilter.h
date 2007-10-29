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

#ifndef _VRKIT_UTIL_REMOTE_ASPECT_FILTER_H_
#define _VRKIT_UTIL_REMOTE_ASPECT_FILTER_H_

#include <map>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>

#include <vrkit/util/RemoteAspectFilterPtr.h>


OSG_BEGIN_NAMESPACE
class RemoteAspect;
OSG_END_NAMESPACE

namespace vrkit
{

namespace util
{

/** \class RemoteAspectFilter RemoteAspectFilter.h vrkit/util/RemoteAspectFilter.h
 *
 * @note This class was moved into the vrkit::util namespace in version 0.47.
 *
 * @since 0.19.0
 */
class RemoteAspectFilter
   : public boost::enable_shared_from_this<RemoteAspectFilter>
{
protected:
   RemoteAspectFilter();

public:
   static RemoteAspectFilterPtr create()
   {
      return RemoteAspectFilterPtr(new RemoteAspectFilter());
   }

   ~RemoteAspectFilter();

   typedef boost::function<void (OSG::FieldContainerPtr)> callback_t;

   void addChangedCallback(OSG::FieldContainerPtr fcp, callback_t callback);
   void addCreatedCallback(OSG::FieldContainerPtr fcp, callback_t callback);
   void addDestroyedCallback(OSG::FieldContainerPtr fcp, callback_t callback);

   /**
    * Registers callback functions with RemoteAspect for all field containers.
    *
    * @return This object is returned as a shared pointer.
    */
   RemoteAspectFilterPtr init(OSG::RemoteAspect* remoteAspect);

   /** @name OpenSG RemoteAspect callbacks */
   //@{
#if OSG_MAJOR_VERSION < 2
   typedef OSG::FieldContainerPtr& field_container_ptr_type;
#else
   typedef OSG::FieldContainerPtrConstArg field_container_ptr_type;
#endif

   bool createdFunction(field_container_ptr_type fcp,
                        OSG::RemoteAspect* aspect);

   bool changedFunction(field_container_ptr_type fcp,
                        OSG::RemoteAspect* aspect);

   bool destroyedFunction(field_container_ptr_type fcp,
                          OSG::RemoteAspect* aspect);
   //@}

protected:
   OSG::RemoteAspect* mRemoteAspect;

   //typedef std::hash_multimap<OSG::FieldContainerPtr, boost::function< void (const vpr::GUID&) >, vpr::GUID::hash> change_callback_map_t;
   typedef std::multimap<OSG::FieldContainerPtr, callback_t> callback_map_t;

   callback_map_t mChangedCallbacks;
   callback_map_t mCreatedCallbacks;
   callback_map_t mDestroyedCallbacks;

   int    mNodes;
   int    mTransforms;
   int    mGeometries;
   int    mMaterials;
};

}

}


#endif /* _VRKIT_UTIL_REMOTE_ASPECT_FILTER_H_ */
