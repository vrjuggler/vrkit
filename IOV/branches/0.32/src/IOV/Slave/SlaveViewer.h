// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_SLAVE_VIEWER_H_
#define _INF_SLAVE_VIEWER_H_

#include <string>

#include <IOV/Config.h>

#include <OpenSG/OSGGroup.h>
#include <OpenSG/OSGNode.h>
#include <OpenSG/OSGCoredNodePtr.h>
#include <OpenSG/OSGRemoteAspect.h>
#include <OpenSG/OSGPointConnection.h>
#include <OpenSG/OSGBinaryDataHandler.h>

#include <vrj/Draw/OpenSG/OpenSGApp.h>


namespace inf
{

/** Viewer class/app for the slave side of cluster communication.
 *
 * This class implements the slave side of the cluster protocol.
 *
 * See: @ref SlaveCommunicationProtocol
 *
 */
class IOV_CLASS_API SlaveViewer : public vrj::OpenSGApp
{
public:
   /**
    * Constructs a new slave viewer application object instance.
    *
    * @param masterAddr   The address of the master node in the cluster in
    *                     the form "address:port" where "address" can be
    *                     either an IP address or a host name.
    * @param rootNodeName The name of the root node of the shared scene graph.
    *                     This parameter is optional, and it defaults to
    *                     "RootNode" if not specified. This must match
    *                     whatever the master inf::Viewer instance was
    *                     configured to use for its shared root node name.
    * @param travMask     The traversal mask to be applied to all OpenSG
    *                     viewport render actions associated with this
    *                     application object instance. This parameter is
    *                     optional and defaults to 0xffffffff if not
    *                     specified.
    *
    * @see inf::Viewer
    */
   SlaveViewer(const std::string& masterAddr,
               const std::string& rootNodeName = "RootNode",
               const OSG::UInt32 travMask = 0xffffffff);

   virtual ~SlaveViewer();

   virtual void initScene();

   virtual OSG::NodePtr getScene()
   {
      return mSceneRoot.node();
   }

   virtual void contextInit();

   virtual void latePreFrame();

   virtual float getDrawScaleFactor()
   {
      return mDrawScaleFactor;
   }

   virtual void exit();

public:
   /**
    * @name Cluster app data methods.
    *
    * These methods are used to communicate data over an OpenSG network
    * connection with the master node. It is called as part of the cluster
    * communication protocol in latePreFrame().
    *
    * @note Derived classes must call up to parent class methods.
    */
   //@{
   virtual void sendDataToMaster(OSG::BinaryDataHandler& writer);

   virtual void readDataFromMaster(OSG::BinaryDataHandler& reader);
   //@}

private:
   void initGl();

   bool createdFunction(OSG::FieldContainerPtr& fcp, OSG::RemoteAspect*);

   bool changedFunction(OSG::FieldContainerPtr& fcp, OSG::RemoteAspect*);

   bool destroyedFunction(OSG::FieldContainerPtr& fcp, OSG::RemoteAspect*);

   void shutdown();

   float mDrawScaleFactor;

   std::string mMasterAddr;
   std::string mRootNodeName;
   OSG::UInt32 mTravMask;       /**< Viewport render action traversal mask */

   OSG::GroupNodePtr mSceneRoot;

   // NOTE: mAspect is allocated dynamically so that we can control when it
   // is deleted.  This is necessary to avoid crash-on-exit problems with
   // OpenSG.
   OSG::RemoteAspect*       mAspect;
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
