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
#include <cstdlib>

#include <OpenSG/OSGAttachmentContainer.h>
#include <OpenSG/OSGConnectionFactory.h>
#include <OpenSG/OSGFieldContainerFactory.h>
#include <OpenSG/OSGNode.h>
#include <OpenSG/OSGThread.h>
#include <OpenSG/OSGTransform.h>
#include <OpenSG/OSGGeometry.h>
#include <OpenSG/OSGSimpleAttachments.h>
#include <OpenSG/OSGSceneFileHandler.h>
#include <OpenSG/OSGVerifyGraphOp.h>

#include <vpr/vpr.h>
#include <vpr/Util/Debug.h>
#include <gadget/Type/Position/PositionUnitConversion.h>

#include <vrkit/ExitCodes.h>
#include <vrkit/slave/SlaveViewer.h>

//OSG_USING_NAMESPACE

/** @page SlaveCommunicationProtocol Slave Communication Protocol
 *
 * The vrkit viewer supports a remote slave protocol. This allows slave
 * applications (usually a subclass of vrkit::SlaveViewer) to connect to the
 * main viewer to get scene graph updates in a cluster configuration. This
 * page documents the communication protocol used for this communication.
 *
 * Notes:
 *   - finish: This is a flag that tells the slave to disconnect when set to
 *             true.
 *
 * <pre>
 * Master                              Slave
 * ------                              ------
 *          ---- Initialization ----
 * bind
 * accept connect (from all slaves)    connect
 * signal                              wait()
 * send(scaleFactor)                   recv(scaleFactor)
 * send(aspect)                        recv(aspect)
 * send(finish)                        recv(finish)
 * flush
 *
 *          ---- Frame Update ----
 * signal                              wait
 * send(aspect)                        recv(aspect)
 * send(finish)                        recv(finish)
 * send(userData), flush               recv(userData)
 * recv(userData) (for each)           send(userData), flush
 * </pre>
 */


namespace
{

const vpr::DebugCategory vrkitSLAVE_APP(
   vpr::GUID("c141b39d-403e-4999-aac0-1102c1471a9f"), "VRKIT_SLAVE_APP",
   "vrkitSLAVE:"
);

const int SLAVE_DBG_LVL(vprDBG_STATE_LVL);

class TravState
{
public:
   TravState()
      : mIndent(0)
   {
      /* Do nothing. */ ;
   }

   OSG::Action::ResultE enter(OSG::NodePtr& node)
   {
      for ( OSG::UInt16 i = 0; i < mIndent; ++i )
      {
         std::cout << "    ";
      }

      std::cout << "entering " << node << "  ";

      const char* node_name = OSG::getName(node);

      if ( NULL == node_name )
      {
         std::cout << "name: <NULL>   ";
      }
      else
      {
         std::cout << "name: " << node_name << "   ";
      }

      std::cout << " type:" << node->getType().getName().str() << " fc_id:"
                << node.getFieldContainerId() << "  ";

      OSG::NodeCorePtr core = node->getCore();
      std::cout << "core: " << core << "  ";

      std::cout << std::endl;

      ++mIndent;

      return OSG::Action::Continue;
   }

   OSG::Action::ResultE leave(OSG::NodePtr& node, OSG::Action::ResultE res)
   {
      --mIndent;

      for ( OSG::UInt16 i = 0; i < mIndent; ++i )
      {
         std::cout << "    ";
      }

      std::cout << "leaving " << node << std::endl;

      // you should return the result that you're passed, to propagate Quits
      return res;
   }

private:
   OSG::UInt16 mIndent;
};

// Nice helper method for printing out the graph.
void printGraph(OSG::NodePtr node)
{
   TravState t;
   std::cout << "------- Scene: root: " << node << " ----------" << std::endl;

   OSG::traverse(
      node,
      OSG::osgTypedMethodFunctor1ObjPtrCPtrRef<
         OSG::Action::ResultE, TravState, OSG::NodePtr
      >(&t, &TravState::enter),
      OSG::osgTypedMethodFunctor2ObjPtrCPtrRef<
         OSG::Action::ResultE, TravState, OSG::NodePtr, OSG::Action::ResultE
      >(&t, &TravState::leave)
   );
   std::cout << "--------------------------------------------" << std::endl;
}

}

namespace vrkit
{

SlaveViewer::SlaveViewer(const std::string& masterAddr,
                         const std::string& rootNodeName,
                         const OSG::UInt32 travMask)
   :
#if __VJ_version >= 2003011
     vrj::opensg::App()
#else
     vrj::OpenSGApp()
#endif
   , mDrawScaleFactor(gadget::PositionUnitConversion::ConvertToFeet)
   , mMasterAddr(masterAddr)
   , mRootNodeName(rootNodeName)
   , mTravMask(travMask)
   , mAspect(new OSG::RemoteAspect())
   , mConnection(NULL)
#ifdef VRKIT_DEBUG
   , mNodes(0)
   , mTransforms(0)
   , mGeometries(0)
#endif
{
   mConnection = OSG::ConnectionFactory::the().createPoint("StreamSock");
}

SlaveViewer::~SlaveViewer()
{
   shutdown();
}

void SlaveViewer::initScene()
{
   OSG::RemoteAspect::Functor changed =
      OSG::osgTypedMethodFunctor2ObjPtrCPtrRef<
         bool, SlaveViewer, OSG::FieldContainerPtr, OSG::RemoteAspect*
      >(this, &SlaveViewer::changedFunction);

   OSG::RemoteAspect::Functor destroyed =
      OSG::osgTypedMethodFunctor2ObjPtrCPtrRef<
         bool, SlaveViewer, OSG::FieldContainerPtr, OSG::RemoteAspect*
      >(this, &SlaveViewer::destroyedFunction);

   OSG::RemoteAspect::Functor created =
      OSG::osgTypedMethodFunctor2ObjPtrCPtrRef<
         bool, SlaveViewer, OSG::FieldContainerPtr, OSG::RemoteAspect*
      >(this, &SlaveViewer::createdFunction);

   for ( OSG::UInt32 i = 0; i < OSG::TypeFactory::the()->getNumTypes(); ++i )
   {
      OSG::FieldContainerType* fct =
         OSG::FieldContainerFactory::the()->findType(i);
      if ( NULL != fct )
      {
         mAspect->registerChanged(*fct, changed);
         mAspect->registerDestroyed(*fct, destroyed);
         mAspect->registerCreated(*fct, created);
      }
   }

   vprDEBUG(vrkitSLAVE_APP, vprDBG_CRITICAL_LVL)
      << "Connecting to master at " << mMasterAddr << std::flush
      << vprDEBUG_FLUSH;
   mChannel = mConnection->connectPoint(mMasterAddr);
   vprDEBUG_CONT(vrkitSLAVE_APP, vprDBG_CRITICAL_LVL)
      << " [OK]" << std::endl << vprDEBUG_FLUSH;

   if ( mChannel != -1 )
   {
      vprDEBUG(vrkitSLAVE_APP, vprDBG_CRITICAL_LVL)
         << "Reading first of scene data: " << std::endl << vprDEBUG_FLUSH;

      mConnection->selectChannel();

      mConnection->wait();
      mConnection->getValue(mDrawScaleFactor);
      mAspect->receiveSync(*mConnection);

      OSG::Thread::getCurrentChangeList()->clearAll();
      OSG::UInt8 finish(false);
      mConnection->getValue(finish);

      /*
      std::cout << "--- Field Containers ---" << std::endl;
      for ( unsigned int i = 0; i < mMaybeNamedFcs.size(); ++i )
      {
         OSG::AttachmentContainerPtr acp = mMaybeNamedFcs[i];

         const char* node_name = OSG::getName(acp);

         if ( NULL == node_name )
         {
            std::cout << i << ": <NULL>" << std::endl;
         }
         else
         {
            std::cout << i << ": " << node_name << std::endl;
         }
      }
      std::cout << "------" << std::endl;
      */

      vprDEBUG(vrkitSLAVE_APP, vprDBG_CRITICAL_LVL)
         << "Searching for scene root (name is " << mRootNodeName
         << ") ... " << std::flush << vprDEBUG_FLUSH;

      const std::vector<OSG::FieldContainerPtr>* fcs =
         OSG::FieldContainerFactory::the()->getFieldContainerStore();

      for ( OSG::UInt32 i = 0; i < fcs->size(); ++i )
      {
         OSG::NodePtr node = OSG::NodePtr::dcast((*fcs)[i]);

         if ( OSG::NullFC != node )
         {
            const char* node_name = OSG::getName(node);

            if ( node_name != NULL && std::string(node_name) == mRootNodeName )
            {
               vprDEBUG_CONT(vrkitSLAVE_APP, vprDBG_CRITICAL_LVL)
                  << "Found it." << std::endl << vprDEBUG_FLUSH;
               mSceneRoot = node;
               break;
            }
         }
      }

      if ( OSG::NullFC == mSceneRoot.node() )
      {
         vprDEBUG_CONT(vrkitSLAVE_APP, vprDBG_CRITICAL_LVL)
            << "NOT FOUND! Exiting with error status -256." << std::endl
            << vprDEBUG_FLUSH;
         std::exit(-256);
      }

      /*
      OSG::NodePtr temp_node = mSceneRoot.node();
      std::cout << "--- Verifying initial scene:" << std::endl;
      OSG::VerifyGraphOp verify_op;
      verify_op.setVerbose(true);
      verify_op.setRepair(true);
      verify_op.traverse(temp_node);

      //printGraph(temp_node);
      */
   }
   else
   {
      std::cerr << "ERROR: Failed to connect to master!" << std::endl;
      OSG::osgExit();
      std::exit(EXIT_ERR_CONNECT_FAIL);
   }
}

void SlaveViewer::contextInit()
{
#if __VJ_version >= 2003011
   vrj::opensg::App::contextInit();
#else
   vrj::OpenSGApp::contextInit();
#endif
   vprDEBUG(vrkitSLAVE_APP, vprDBG_STATE_LVL)
      << "Changing render action traversal mask from " << std::hex
      << mContextData->mRenderAction->getTravMask() << " to " << mTravMask
      << std::dec << std::endl << vprDEBUG_FLUSH;

   mContextData->mRenderAction->setTravMask(mTravMask);
   OSG::beginEditCP(mContextData->mViewport,
                    OSG::Viewport::TravMaskFieldMask);
      mContextData->mViewport->setTravMask(mTravMask);
   OSG::endEditCP(mContextData->mViewport, OSG::Viewport::TravMaskFieldMask);

   initGl();
}

void SlaveViewer::latePreFrame()
{
//   static int iter_num(0);
   try
   {
      OSG::UInt8 finish(false);

      if ( mConnection->wait() )
      {
         //std::cout << "-------- Recv data: " << iter_num++ << " --------"
         //          << std::endl;
         mAspect->receiveSync(*mConnection);
         OSG::Thread::getCurrentChangeList()->clearAll();
         mConnection->getValue(finish);
         readDataFromMaster(*mConnection);
         sendDataToMaster(*mConnection);
         mConnection->flush();

         /*
         OSG::NodePtr temp_node = mSceneRoot.node();
         std::cout << "--- Verifying Graph:" << std::endl;
         OSG::VerifyGraphOp verify_op;
         verify_op.setVerbose(true);
         verify_op.setRepair(true);
         verify_op.traverse(temp_node);
         std::cout << "-- Verify complete:" << std::endl;
         */

         //printGraph(temp_node);
      }

      if ( finish )
      {
         mConnection->putValue(finish);
         mConnection->flush();

         shutdown();

         OSG::osgExit();
         ::exit(EXIT_SUCCESS);
      }
   }
   catch (OSG::Exception& ex)
   {
      std::cerr << ex.what() << std::endl;
      shutdown();

      OSG::osgExit();
      std::exit(EXIT_ERR_COMM);
   }

   /*
   std::string file_name;
   if ( iter_num % 3 == 0 )
   {
      file_name = std::string("slave_scene.out0.osb");
   }
   else if ( iter_num % 3 == 1 )
   {
      file_name = std::string("slave_scene.out1.osb");
   }
   else if ( iter_num % 3 == 2 )
   {
      file_name = std::string("slave_scene.out2.osb");
   }

   OSG::SceneFileHandler::the().write(getScene(), file_name.c_str());
   */
}

void SlaveViewer::exit()
{
   mSceneRoot = OSG::NullFC;
   shutdown();
   mMaybeNamedFcs.clear();
#if __VJ_version >= 2003011
   vrj::opensg::App::exit();
#else
   vrj::OpenSGApp::exit();
#endif
}

void SlaveViewer::sendDataToMaster(OSG::BinaryDataHandler& writer)
{
   OSG::UInt8 junk(false);
   writer.putValue(junk);
}

void SlaveViewer::readDataFromMaster(OSG::BinaryDataHandler& reader)
{
   OSG::UInt8 junk;
   reader.getValue(junk);
   float near_val, far_val;
   reader.getValue(near_val);
   reader.getValue(far_val);
   vrj::Projection::setNearFar(near_val, far_val);
}

void SlaveViewer::initGl()
{
   // VR Juggler 2.1.25 and beyond perform the necessary lighting of simulator
   // components, so we do not have to do it ourselves.
#if __VJ_version >= 2001025
   glEnable(GL_NORMALIZE);
#else
   GLfloat light0_ambient[]  = { 0.1f,  0.1f,  0.1f, 1.0f };
   GLfloat light0_diffuse[]  = { 0.7f,  0.7f,  0.7f, 1.0f };
   GLfloat light0_specular[] = { 1.0f,  1.0f,  1.0f, 1.0f };
   GLfloat light0_position[] = { 0.5f, 0.75f, 0.75f, 0.0f };

   GLfloat mat_ambient[]   = { 0.7f, 0.7f, 0.7f, 1.0f };
   GLfloat mat_diffuse[]   = { 1.0f, 0.5f, 0.8f, 1.0f };
   GLfloat mat_specular[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
   GLfloat mat_shininess[] = { 50.0f };
//   GLfloat mat_emission[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
   GLfloat no_mat[]        = { 0.0f, 0.0f, 0.0f, 1.0f };

   glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
   glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_diffuse);
   glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);
   glLightfv(GL_LIGHT0, GL_POSITION, light0_position);

   glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
   glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
   glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
   glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
   glMaterialfv(GL_FRONT, GL_EMISSION, no_mat);

   glEnable(GL_DEPTH_TEST);
   glEnable(GL_NORMALIZE);
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);
   glEnable(GL_COLOR_MATERIAL);
   glShadeModel(GL_SMOOTH);
#endif
}

bool SlaveViewer::createdFunction(OSG::FieldContainerPtr& fcp,
                                  OSG::RemoteAspect*)
{
#ifdef VRKIT_DEBUG
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

   vprDEBUG(vrkitSLAVE_APP, SLAVE_DBG_LVL)
      << "Created: " << fcp->getType().getName() << " fc_id:"
      << fcp.getFieldContainerId() << " " << vprDEBUG_FLUSH;
#endif

   OSG::AttachmentContainerPtr acp = OSG::AttachmentContainerPtr::dcast(fcp);

   if ( OSG::NullFC != acp )
   {
#ifdef VRKIT_DEBUG
      const char* node_name = OSG::getName(acp);

      if ( NULL == node_name )
      {
         vprDEBUG_CONT(vrkitSLAVE_APP, SLAVE_DBG_LVL)
            << "name: <NULL>" << vprDEBUG_FLUSH;
      }
      else
      {
         vprDEBUG_CONT(vrkitSLAVE_APP, SLAVE_DBG_LVL)
            << "name: " << node_name << vprDEBUG_FLUSH;
      }
#endif

      mMaybeNamedFcs.push_back(acp);
   }

#ifdef VRKIT_DEBUG
   vprDEBUG_CONT(vrkitSLAVE_APP, SLAVE_DBG_LVL)
      << std::endl << vprDEBUG_FLUSH;
#endif

   return true;
}

bool SlaveViewer::changedFunction(OSG::FieldContainerPtr& fcp,
                                  OSG::RemoteAspect*)
{
#ifdef VRKIT_DEBUG
   vprDEBUG(vrkitSLAVE_APP, SLAVE_DBG_LVL)
      << "Changed: " << fcp->getType().getName() << " fc_id:"
      << fcp.getFieldContainerId() << " "<< vprDEBUG_FLUSH;

   OSG::AttachmentContainerPtr acp = OSG::AttachmentContainerPtr::dcast(fcp);

   if ( OSG::NullFC != acp )
   {
      const char* node_name = OSG::getName(acp);
      if ( NULL == node_name )
      {
         vprDEBUG_CONT(vrkitSLAVE_APP, SLAVE_DBG_LVL)
            << "name: <NULL>" << std::endl << vprDEBUG_FLUSH;
      }
      else
      {
         vprDEBUG_CONT(vrkitSLAVE_APP, SLAVE_DBG_LVL)
            << "name: " << node_name << std::endl << vprDEBUG_FLUSH;
      }
   }
   else
   {
      vprDEBUG_CONT(vrkitSLAVE_APP, SLAVE_DBG_LVL)
         << std::endl << vprDEBUG_FLUSH;
   }
#endif

   return true;
}

bool SlaveViewer::destroyedFunction(OSG::FieldContainerPtr& fcp,
                                    OSG::RemoteAspect*)
{
#ifdef VRKIT_DEBUG
   vprDEBUG(vrkitSLAVE_APP, SLAVE_DBG_LVL)
      << "Destroyed: " << fcp->getType().getName() << " fc_id:"
      << fcp.getFieldContainerId() << " "<< vprDEBUG_FLUSH;

   OSG::AttachmentContainerPtr acp = OSG::AttachmentContainerPtr::dcast(fcp);

   if ( OSG::NullFC != acp )
   {
      const char* node_name = OSG::getName(acp);
      if ( NULL == node_name )
      {
         vprDEBUG_CONT(vrkitSLAVE_APP, SLAVE_DBG_LVL)
            << "name: <NULL>" << std::endl << vprDEBUG_FLUSH;
      }
      else
      {
         vprDEBUG_CONT(vrkitSLAVE_APP, SLAVE_DBG_LVL)
            << "name: " << node_name << std::endl << vprDEBUG_FLUSH;
      }
   }
   else
   {
      vprDEBUG_CONT(vrkitSLAVE_APP, SLAVE_DBG_LVL)
         << std::endl << vprDEBUG_FLUSH;
   }

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
#endif

   return true;
}

void SlaveViewer::shutdown()
{
   if ( NULL != mConnection )
   {
      mConnection->disconnect();
      delete mConnection;
      mConnection = NULL;
   }

   if ( NULL != mAspect )
   {
      delete mAspect;
      mAspect = NULL;
   }
}

}
