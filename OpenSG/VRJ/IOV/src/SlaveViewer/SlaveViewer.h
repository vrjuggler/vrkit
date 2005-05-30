#ifndef _INF_SLAVE_VIEWER_H_
#define _INF_SLAVE_VIEWER_H_

#include <string>

#include <OpenSG/OSGGroup.h>
#include <OpenSG/OSGNode.h>
#include <OpenSG/OSGCoredNodePtr.h>
#include <OpenSG/OSGRemoteAspect.h>
#include <OpenSG/OSGPointConnection.h>

#include <vrj/Draw/OpenSG/OpenSGApp.h>


namespace inf
{

typedef OSG::CoredNodePtr<OSG::Group> CoredGroupPtr;


/** Viewer class/app for the slave side of cluster communication.
 *
 * This class implements the slave side of the cluster protocol.
 *
 * See: @ref SlaveCommunicationProtocol
 *
 */
class SlaveViewer : public vrj::OpenSGApp
{
public:
   SlaveViewer(const std::string& masterAddr,
               const std::string& rootNodeName = "RootNode");

   virtual ~SlaveViewer();

   virtual void initScene();

   virtual OSG::NodePtr getScene()
   {
      return mSceneRoot.node();
   }

   virtual void contextInit();

   virtual void preFrame();

   virtual float getDrawScaleFactor()
   {
      return mDrawScaleFactor;
   }

private:
   void initGl();

   bool createdFunction(OSG::FieldContainerPtr& fcp, OSG::RemoteAspect*);

   bool changedFunction(OSG::FieldContainerPtr& fcp, OSG::RemoteAspect*);

   bool destroyedFunction(OSG::FieldContainerPtr& fcp, OSG::RemoteAspect*);

   void shutdown();

   float mDrawScaleFactor;

   std::string mMasterAddr;
   std::string mRootNodeName;

   CoredGroupPtr mSceneRoot;

   OSG::RemoteAspect        mAspect;
   OSG::PointConnection*    mConnection;
   OSG::Connection::Channel mChannel;

   std::vector<OSG::AttachmentContainerPtr> mMaybeNamedFcs;

#ifdef _DEBUG
   unsigned int mNodes;
   unsigned int mTransforms;
   unsigned int mGeometries;
#endif
};

}

#endif
