// Copyright (C) Infiscape Corporation 2005-2006

#include <iostream>
#include <stdlib.h>

#include <OpenSG/OSGAttachmentContainer.h>
#include <OpenSG/OSGConnectionFactory.h>
#include <OpenSG/OSGConnectionType.h>
#include <OpenSG/OSGFieldContainerFactory.h>
#include <OpenSG/OSGTypeFactory.h>
#include <OpenSG/OSGContainerFactoryMixins.h>
#include <OpenSG/OSGNode.h>
#include <OpenSG/OSGThread.h>
#include <OpenSG/OSGTransform.h>
#include <OpenSG/OSGGeometry.h>
#include <OpenSG/OSGNameAttachment.h>
#include <OpenSG/OSGSceneFileHandler.h>
#include <OpenSG/OSGVerifyGraphOp.h>

#include <vpr/vpr.h>
#include <vpr/Util/Debug.h>
#include <gadget/Type/Position/PositionUnitConversion.h>

#include <IOV/ExitCodes.h>
#include <IOV/Slave/SlaveViewer.h>

//OSG_USING_NAMESPACE

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
 * send(userData), flush               recv(userData)
 * recv(userData) (for each)           send(userData), flush
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


class travstate
{
  public:

    travstate( void ) : _indent(0) {}

    OSG::Action::ResultE enter(OSG::NodePtrConstArg node)
    {
        for(OSG::UInt16 i = 0; i < _indent; i++)
        { std::cout << "    "; }

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

        std::cout << " type:" << node->getType().getName().str()
		  << " fc_id:" << node->getType().getId() << "  ";

        OSG::NodeCorePtr core = node->getCore();
        std::cout << "core: " << core << "  ";

        std::cout << std::endl;

        ++_indent;
        return OSG::Action::Continue;
    }

    OSG::Action::ResultE leave(OSG::NodePtrConstArg node, OSG::Action::ResultE res)
    {
        --_indent;

        for(OSG::UInt16 i = 0; i < _indent; i++)
        { std::cout << "    "; }

        std::cout << "leaving " << node << std::endl;

        // you should return the result that you're passed, to propagate Quits
        return res;
    }

  private:

    OSG::UInt16 _indent;
};

// Nice helper method for printing out the graph.
void printGraph(OSG::NodePtr node)
{
   travstate t;
   std::cout << "------- Scene: root: " << node << " ----------" << std::endl;

#ifndef OPENSG2SHIM
   OSG::traverse(node,
             OSG::osgTypedMethodFunctor1ObjPtrCPtrRef<OSG::Action::ResultE,
                                                 travstate,
                                                 OSG::NodePtr>(
                                                     &t,
                                                     &travstate::enter),
             OSG::osgTypedMethodFunctor2ObjPtrCPtrRef<OSG::Action::ResultE,
                                                 travstate,
                                                 OSG::NodePtr,
                                                 OSG::Action::ResultE>(
                                                     &t,
                                                     &travstate::leave));
#else
   OSG::TraverseEnterFunctor f1 = boost::bind(&travstate::enter,
					     &t, _1);
   OSG::TraverseLeaveFunctor f2 = boost::bind(&travstate::leave,
					     &t, _1, _2);
   OSG::traverse(node, f1, f2);
#endif
   std::cout << "--------------------------------------------" << std::endl;
}

}

namespace inf
{

SlaveViewer::SlaveViewer(const std::string& masterAddr,
                         const std::string& rootNodeName)
   : vrj::OpenSGApp()
   , mDrawScaleFactor(gadget::PositionUnitConversion::ConvertToFeet)
   , mMasterAddr(masterAddr)
   , mRootNodeName(rootNodeName)
   , mAspect(new OSG::RemoteAspect())
   , mConnection(NULL)
#ifdef _DEBUG
   , mNodes(0)
   , mTransforms(0)
   , mGeometries(0)
#endif
{
   mConnection = OSG::ConnectionFactory::the()->createPoint("StreamSock");
}

SlaveViewer::~SlaveViewer()
{
   shutdown();
}

void SlaveViewer::initScene()
{
#ifndef OPENSG2SHIM
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
#else
   OSG::RemoteAspect::Functor changed = boost::bind(&SlaveViewer::changedFunction,
					     this, _1, _2);

   OSG::RemoteAspect::Functor destroyed = boost::bind(&SlaveViewer::destroyedFunction,
					     this, _1, _2);

   OSG::RemoteAspect::Functor created = boost::bind(&SlaveViewer::createdFunction,
					     this, _1, _2);
#endif

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

   std::cout << "Slave: Connecting to addr: " << mMasterAddr << std::flush;
   mChannel = mConnection->connectPoint(mMasterAddr);
   std::cout << "[OK]" << std::endl;

   if ( mChannel != -1 )
   {
      std::cout << "Slave: Reading first of scene data: " << std::endl;

      mConnection->selectChannel();

      mConnection->wait();
      mConnection->getValue(mDrawScaleFactor);
      mAspect->receiveSync(*mConnection);

      OSG::Thread::getCurrentChangeList()->clear();
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

      std::cout << "--- Searching for scene root (name is " << mRootNodeName
                << ") ---" << std::endl;

      const std::vector<OSG::FieldContainerPtr> fcs =
         OSG::FieldContainerFactory::the()->getContainerStore();

      for ( OSG::UInt32 i = 0; i < fcs.size(); ++i )
      {
         OSG::NodePtr node = OSG::cast_dynamic<OSG::NodePtr>((fcs)[i]);

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

      OSG::NodePtr temp_node = mSceneRoot.node();
      /*
      std::cout << "--- Verifying initial scene:" << std::endl;
      OSG::VerifyGraphOp verify_op;
      verify_op.setVerbose(true);
      verify_op.setRepair(true);
      verify_op.traverse(temp_node);
      */

      //printGraph(temp_node);

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
//   static int iter_num(0);
   try
   {
      OSG::UInt8 finish(false);

      if ( mConnection->wait() )
      {
         //std::cout << "-------- Recv data: " << iter_num++ << " --------"
         //          << std::endl;
         mAspect->receiveSync(*mConnection);
         OSG::Thread::getCurrentChangeList()->clear();
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
      ::exit(inf::EXIT_ERR_COMM);
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
   vrj::OpenSGApp::exit();
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

bool SlaveViewer::createdFunction(OSG::FieldContainerPtrConstArg fcp,
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
             << fcp->getType().getId() << " " << vprDEBUG_FLUSH;
#endif

   OSG::AttachmentContainerPtr acp = OSG::cast_dynamic<OSG::AttachmentContainerPtr>(fcp);

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

bool SlaveViewer::changedFunction(OSG::FieldContainerPtrConstArg fcp,
                                  OSG::RemoteAspect*)
{
#ifdef _DEBUG
   vprDEBUG(infSLAVE_APP, SLAVE_DBG_LVL)
      << "Changed: " << fcp->getType().getName().str() << " fc_id:"
             << fcp->getType().getId() << " "<< vprDEBUG_FLUSH;

   OSG::AttachmentContainerPtr acp = OSG::cast_dynamic<OSG::AttachmentContainerPtr>(fcp);

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

bool SlaveViewer::destroyedFunction(OSG::FieldContainerPtrConstArg fcp,
                                    OSG::RemoteAspect*)
{
#ifdef _DEBUG
   vprDEBUG(infSLAVE_APP, SLAVE_DBG_LVL)
      << "Destroyed: " << fcp->getType().getName().str() << " fc_id:"
             << fcp->getType().getId() << " "<< vprDEBUG_FLUSH;

   OSG::AttachmentContainerPtr acp = OSG::cast_dynamic<OSG::AttachmentContainerPtr>(fcp);

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
      mConnection = NULL;
   }

   if ( NULL != mAspect )
   {
      delete mAspect;
      mAspect = NULL;
   }
}

}
