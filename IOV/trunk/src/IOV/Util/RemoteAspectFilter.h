// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_REMOTE_ASPECT_FILTER_H_
#define _INF_REMOTE_ASPECT_FILTER_H_

#include <map>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>

#include <OpenSG/OSGRemoteAspect.h>

#include <IOV/Util/RemoteAspectFilterPtr.h>


namespace inf
{

/**
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

   void addChangedCallback(OSG::FieldContainerPtr fcp, boost::function< void (OSG::FieldContainerPtr)> callback);
   void addCreatedCallback(OSG::FieldContainerPtr fcp, boost::function< void (OSG::FieldContainerPtr)> callback);
   void addDestroyedCallback(OSG::FieldContainerPtr fcp, boost::function< void (OSG::FieldContainerPtr)> callback);

   /**
    * Registers callback functions with RemoteAspect for all field containers.
    *
    * @return This object is returned as a shared pointer.
    */
   RemoteAspectFilterPtr init(OSG::RemoteAspect* remoteAspect);

   /** @name OpenSG RemoteAspect callbacks */
   //@{
   bool createdFunction(OSG::FieldContainerPtr &fcp, OSG::RemoteAspect *);
   bool destroyedFunction(OSG::FieldContainerPtr &fcp, OSG::RemoteAspect *);
   bool changedFunction(OSG::FieldContainerPtr &fcp, OSG::RemoteAspect *);
   //@}
protected:
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
