// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_REMOTE_ASPECT_FILTER_H_
#define _INF_REMOTE_ASPECT_FILTER_H_

#include <string>
#include <vector>
#include <boost/enable_shared_from_this.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/signals/connection.hpp>

#include <OpenSG/OSGColor.h>
#include <OpenSG/OSGRemoteAspect.h>
#include <OpenSG/OSGTypedFunctors.h>

#include <jccl/Config/ConfigElementPtr.h>

#include <IOV/ViewerPtr.h>
#include <IOV/SceneObjectPtr.h>
#include <IOV/GeometryHighlightTraverser.h>
#include <IOV/Util/RemoteAspectFilterPtr.h>

#include <boost/function.hpp>

namespace inf
{

/**
 * @since 0.19.0
 */
class RemoteAspectFilter
   : public boost::enable_shared_from_this<RemoteAspectFilter>
{
public:
   static RemoteAspectFilterPtr create()
   {
      return RemoteAspectFilterPtr(new RemoteAspectFilter());
   }

   ~RemoteAspectFilter();

   void addChangedCallback(OSG::FieldContainerPtr fcp, boost::function< void (OSG::FieldContainerPtr)> callback);
   void addCreatedCallback(OSG::FieldContainerPtr fcp, boost::function< void (OSG::FieldContainerPtr)> callback);
   void addDestroyedCallback(OSG::FieldContainerPtr fcp, boost::function< void (OSG::FieldContainerPtr)> callback);
   
   /** Registers callback functions with RemoteAspect for all field containers. */
   void init(OSG::RemoteAspect* remoteAspect);

   /** @name OpenSG RemoteAspect callbacks */
   //@{
   bool createdFunction(OSG::FieldContainerPtr &fcp, OSG::RemoteAspect *);
   bool destroyedFunction(OSG::FieldContainerPtr &fcp, OSG::RemoteAspect *);
   bool changedFunction(OSG::FieldContainerPtr &fcp, OSG::RemoteAspect *);
   //@}
protected:
   RemoteAspectFilter();
   OSG::RemoteAspect* mRemoteAspect;

   //typedef std::hash_multimap<OSG::FieldContainerPtr, boost::function< void (const vpr::GUID&) >, vpr::GUID::hash> change_callback_map_t;
   typedef std::multimap<OSG::FieldContainerPtr, boost::function< void (OSG::FieldContainerPtr) > > callback_map_t;

   callback_map_t mChangedCallbacks;
   callback_map_t mCreatedCallbacks;
   callback_map_t mDestroyedCallbacks;
   
   int    mNodes;
   int    mTransforms;
   int    mGeometries;
   int    mMaterials;
private:
};

}


#endif /* _INF_REMOTE_ASPECT_FILTER_H_ */
