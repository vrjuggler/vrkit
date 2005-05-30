#include <iostream>
#include <stdlib.h>

#include <OpenSG/OSGAttachmentContainer.h>
#include <OpenSG/OSGConnectionFactory.h>
#include <OpenSG/OSGFieldContainerFactory.h>
#include <OpenSG/OSGNode.h>
#include <OpenSG/OSGThread.h>
#include <OpenSG/OSGTransform.h>
#include <OpenSG/OSGGeometry.h>
#include <OpenSG/OSGSimpleAttachments.h>

#include <vpr/vpr.h>
#include <vpr/Util/Debug.h>
#include <gadget/Type/Position/PositionUnitConversion.h>

#include <IOV/ExitCodes.h>
#include <IOV/Slave/SlaveViewer.h>

/** @page SlaveCommunicationProtocol Slave Communication Protocol
 *
 * The IOV viewer supports a remote slave protocol.  This allows slave
 * applications to connect to the main viewer to get scene graph
 * updates in a cluster configuration.
 *
 * This page documents the communication protocol used for this comm.
 *
 * Notes:
 * - finish - This is a flag that is true when the slave should disconnect.
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
 * flush                               signal()
 * wait()                              flush()
 * </pre>
 *
 */


namespace
{

const vpr::DebugCategory infSLAVE_APP(
   vpr::GUID("c141b39d-403e-4999-aac0-1102c1471a9f"), "INF_SLAVE_APP",
   "SLAVE_APP:"
);

const int SLAVE_DBG_LVL(vprDBG_STATE_LVL);
//const int SLAVE_DBG_LVL(0);

}

namespace inf
{

SlaveViewer::SlaveViewer(const std::string& masterAddr,
                         const std::string& rootNodeName)
   : vrj::OpenSGApp()
   , mDrawScaleFactor(gadget::PositionUnitConversion::ConvertToFeet)
   , mMasterAddr(masterAddr)
   , mRootNodeName(rootNodeName)
   , mConnection(NULL)
#ifdef _DEBUG
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
         mAspect.registerChanged(*fct, changed);
         mAspect.registerDestroyed(*fct, destroyed);
         mAspect.registerCreated(*fct, created);
      }
   }

   std::cout << "Slave: Connecting to addr: " << mMasterAddr << std::flush;
   mChannel = mConnection->connectPoint(mMasterAddr);
   std::cout << "[OK]" << std::endl;

   if ( mChannel != -1 )
   {
      mConnection->selectChannel();

      mConnection->wait();
      mConnection->getValue(mDrawScaleFactor);
      mAspect.receiveSync(*mConnection);

      OSG::Thread::getCurrentChangeList()->clearAll();
      OSG::UInt8 finish(false);
      mConnection->getValue(finish);

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

      std::cout << "--- Searching for scene root (name is " << mRootNodeName
                << ") ---" << std::endl;

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
               std::cout << "Found it." << std::endl;
               mSceneRoot = node;
               break;
            }
         }
      }

      if ( OSG::NullFC == mSceneRoot.node() )
      {
         std::cout << "NOT FOUND." << std::endl;
         ::exit(-256);
      }

      std::cout << "------" << std::endl;
   }
   else
   {
      std::cerr << "ERROR: Failed to connect to master!" << std::endl;
      OSG::osgExit();
      ::exit(inf::EXIT_ERR_CONNECT_FAIL);
   }
}

void SlaveViewer::contextInit()
{
   vrj::OpenSGApp::contextInit();
   initGl();
}

void SlaveViewer::latePreFrame()
{
   try
   {
      OSG::UInt8 finish(false);

      if ( mConnection->wait() )
      {
         mAspect.receiveSync(*mConnection);
         OSG::Thread::getCurrentChangeList()->clearAll();
         mConnection->getValue(finish);
         mConnection->signal();
         mConnection->flush();
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
      ::exit(inf::EXIT_ERR_COMM);
   }
}

void SlaveViewer::initGl()
{
   //glEnable(GL_NORMALIZE);

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
}

bool SlaveViewer::createdFunction(OSG::FieldContainerPtr& fcp,
                                  OSG::RemoteAspect*)
{
#ifdef _DEBUG
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

   vprDEBUG(infSLAVE_APP, SLAVE_DBG_LVL) << "Created: " << fcp->getType().getName().str() << " fc_id:"
             << fcp.getFieldContainerId() << " " << vprDEBUG_FLUSH;
#endif

   OSG::AttachmentContainerPtr acp = OSG::AttachmentContainerPtr::dcast(fcp);

   if ( OSG::NullFC != acp )
   {
#ifdef _DEBUG
      const char* node_name = OSG::getName(acp);

      if ( NULL == node_name )
      {
         vprDEBUG_CONT(infSLAVE_APP, SLAVE_DBG_LVL) << "name: <NULL>" << vprDEBUG_FLUSH;
      }
      else
      {
         vprDEBUG_CONT(infSLAVE_APP, SLAVE_DBG_LVL) << "name: " << node_name << vprDEBUG_FLUSH;
      }
#endif

      mMaybeNamedFcs.push_back(acp);
   }

#ifdef _DEBUG
   vprDEBUG_CONT(infSLAVE_APP, SLAVE_DBG_LVL) << std::endl << vprDEBUG_FLUSH;
#endif

   return true;
}

bool SlaveViewer::changedFunction(OSG::FieldContainerPtr& fcp,
                                  OSG::RemoteAspect*)
{
#ifdef _DEBUG
   vprDEBUG(infSLAVE_APP, SLAVE_DBG_LVL)
      << "Changed: " << fcp->getType().getName().str() << " fc_id:"
             << fcp.getFieldContainerId() << " "<< vprDEBUG_FLUSH;

   OSG::AttachmentContainerPtr acp = OSG::AttachmentContainerPtr::dcast(fcp);

   if ( OSG::NullFC != acp )
   {
      const char* node_name = OSG::getName(acp);
      if ( NULL == node_name )
      {
         vprDEBUG_CONT(infSLAVE_APP, SLAVE_DBG_LVL) << "name: <NULL>" << std::endl << vprDEBUG_FLUSH;
      }
      else
      {
         vprDEBUG_CONT(infSLAVE_APP, SLAVE_DBG_LVL) << "name: " << node_name
                                                  << std::endl << vprDEBUG_FLUSH;
      }
   }
   else
   {
      vprDEBUG_CONT(infSLAVE_APP, SLAVE_DBG_LVL) << std::endl << vprDEBUG_FLUSH;
   }
#endif

   return true;
}

bool SlaveViewer::destroyedFunction(OSG::FieldContainerPtr& fcp,
                                    OSG::RemoteAspect*)
{
#ifdef _DEBUG
   vprDEBUG(infSLAVE_APP, SLAVE_DBG_LVL)
      << "Destroyed: " << fcp->getType().getName().str() << " fc_id:"
             << fcp.getFieldContainerId() << " "<< vprDEBUG_FLUSH;

   OSG::AttachmentContainerPtr acp = OSG::AttachmentContainerPtr::dcast(fcp);

   if ( OSG::NullFC != acp )
   {
      const char* node_name = OSG::getName(acp);
      if ( NULL == node_name )
      {
         vprDEBUG_CONT(infSLAVE_APP, SLAVE_DBG_LVL) << "name: <NULL>" << std::endl << vprDEBUG_FLUSH;
      }
      else
      {
         vprDEBUG_CONT(infSLAVE_APP, SLAVE_DBG_LVL) << "name: " << node_name
                                                  << std::endl << vprDEBUG_FLUSH;
      }
   }
   else
   {
      vprDEBUG_CONT(infSLAVE_APP, SLAVE_DBG_LVL) << std::endl << vprDEBUG_FLUSH;
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
   }
}

}
