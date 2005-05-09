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

   virtual void preFrame();

private:
   void shutdown();

   const int EXIT_ERR_CONNECT_FAIL;
   const int EXIT_ERR_COMM;

   std::string mMasterAddr;
   std::string mRootNodeName;

   CoredGroupPtr mSceneRoot;

   OSG::RemoteAspect        mAspect;
   OSG::PointConnection*    mConnection;
   OSG::Connection::Channel mChannel;
};

}

#endif
