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

#ifndef _VRKIT_SLAVE_VIEWER_H_
#define _VRKIT_SLAVE_VIEWER_H_

#include <string>

#include <vrkit/Config.h>

#include <OpenSG/OSGGroup.h>
#include <OpenSG/OSGNode.h>
#include <OpenSG/OSGCoredNodePtr.h>
#include <OpenSG/OSGRemoteAspect.h>
#include <OpenSG/OSGPointConnection.h>
#include <OpenSG/OSGBinaryDataHandler.h>

#include <vrj/vrjParam.h>

#if __VJ_version >= 2003011
#  include <vrj/Draw/OpenSG/App.h>
#else
#  include <vrj/Draw/OpenSG/OpenSGApp.h>
#endif


namespace vrkit
{

/** Viewer class/app for the slave side of cluster communication.
 *
 * This class implements the slave side of the cluster protocol.
 *
 * See: @ref SlaveCommunicationProtocol
 *
 */
class VRKIT_CLASS_API SlaveViewer
#if __VJ_version >= 2003011
   : public vrj::opensg::App
#else
   : public vrj::OpenSGApp
#endif
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
    *                     whatever the master vrkit::Viewer instance was
    *                     configured to use for its shared root node name.
    * @param travMask     The traversal mask to be applied to all OpenSG
    *                     viewport render actions associated with this
    *                     application object instance. This parameter is
    *                     optional and defaults to 0xffffffff if not
    *                     specified.
    *
    * @see vrkit::Viewer
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

#ifdef VRKIT_DEBUG
   unsigned int mNodes;
   unsigned int mTransforms;
   unsigned int mGeometries;
#endif
};

}


#endif /* _VRKIT_SLAVE_VIEWER_H_ */
