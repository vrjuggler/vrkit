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

#include <OpenSG/VRJ/SlaveViewer/exitcodes.h>
#include <OpenSG/VRJ/SlaveViewer/SlaveViewer.h>


namespace
{

const vpr::DebugCategory infSLAVE_APP(
   vpr::GUID("c141b39d-403e-4999-aac0-1102c1471a9f"), "INF_SLAVE_APP",
   "SLAVE_APP:"
);

std::vector<OSG::AttachmentContainerPtr> gMaybeNamedFcs;

#ifdef _DEBUG
unsigned int gNodes(0);
unsigned int gTransforms(0);
unsigned int gGeometries(0);
#endif

bool changedFunction(OSG::FieldContainerPtr& fcp, OSG::RemoteAspect*)
{
#ifdef _DEBUG
   vprDEBUG(infSLAVE_APP, vprDBG_STATE_LVL)
      << "Changed: " << fcp->getType().getName().str() << " "
      << fcp.getFieldContainerId() << std::endl;

   OSG::AttachmentContainerPtr acp = OSG::AttachmentContainerPtr::dcast(fcp);

   if ( OSG::NullFC != acp )
   {
      const char* node_name = OSG::getName(acp);
      if ( NULL == node_name )
      {
         vprDEBUG(infSLAVE_APP, vprDBG_STATE_LVL) << "<NULL>" << std::endl;
      }
      else
      {
         vprDEBUG(infSLAVE_APP, vprDBG_STATE_LVL) << "\tname: " << node_name
                                                  << std::endl;
      }
   }
#endif

   return true;
}

bool createdFunction(OSG::FieldContainerPtr& fcp, OSG::RemoteAspect*)
{
#ifdef _DEBUG
   if ( OSG::Node::getClassType() == fcp->getType() )
   {
      ++gNodes;
   }
   else if ( OSG::Transform::getClassType() == fcp->getType() )
   {
      ++gTransforms;
   }
   else if ( OSG::Geometry::getClassType() == fcp->getType() )
   {
      ++gGeometries;
   }

   std::cout << "Created: " << fcp->getType().getName().str() << " "
             << fcp.getFieldContainerId() << std::endl;
#endif

   OSG::AttachmentContainerPtr acp = OSG::AttachmentContainerPtr::dcast(fcp);

   if ( OSG::NullFC != acp )
   {
#ifdef _DEBUG
      const char* node_name = OSG::getName(acp);

      if ( NULL == node_name )
      {
         std::cout << "<NULL>" << std::endl;
      }
      else
      {
         std::cout << "\tname: " << node_name << std::endl;
      }
#endif

      gMaybeNamedFcs.push_back(acp);
   }

   return true;
}

bool destroyedFunction(OSG::FieldContainerPtr& fcp, OSG::RemoteAspect*)
{
#ifdef _DEBUG
   if ( OSG::Node::getClassType() == fcp->getType() )
   {
      --gNodes;
   }
   else if ( OSG::Transform::getClassType() == fcp->getType() )
   {
      --gTransforms;
   }
   else if ( OSG::Geometry::getClassType() == fcp->getType() )
   {
      --gGeometries;
   }
#endif

   return true;
}

}

namespace inf
{

SlaveViewer::SlaveViewer(const std::string& masterAddr,
                         const std::string& rootNodeName)
   : vrj::OpenSGApp()
   , mMasterAddr(masterAddr)
   , mRootNodeName(rootNodeName)
   , mConnection(NULL)
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
      OSG::osgTypedFunctionFunctor2CPtrRef<bool, OSG::FieldContainerPtr, OSG::RemoteAspect*>(changedFunction);
   OSG::RemoteAspect::Functor destroyed =
      OSG::osgTypedFunctionFunctor2CPtrRef<bool, OSG::FieldContainerPtr, OSG::RemoteAspect*>(destroyedFunction);
   OSG::RemoteAspect::Functor created =
      OSG::osgTypedFunctionFunctor2CPtrRef<bool, OSG::FieldContainerPtr, OSG::RemoteAspect*>(createdFunction);

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

   mChannel = mConnection->connectPoint(mMasterAddr);

   if ( mChannel != -1 )
   {
      mConnection->selectChannel();

      mConnection->wait();
      mAspect.receiveSync(*mConnection);

      OSG::Thread::getCurrentChangeList()->clearAll();
      int finish(0);
      mConnection->getValue(finish);

      std::cout << "--- Field Containers ---" << std::endl;
      for ( unsigned int i = 0; i < gMaybeNamedFcs.size(); ++i )
      {
         OSG::AttachmentContainerPtr acp = gMaybeNamedFcs[i];

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

void SlaveViewer::preFrame()
{
   try
   {
      int finish(0);

      if ( mConnection->wait(0) )
      {
         mAspect.receiveSync(*mConnection);
         OSG::Thread::getCurrentChangeList()->clearAll();
         mConnection->getValue(finish);
      }

      if ( finish != 0 )
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

void SlaveViewer::shutdown()
{
   if ( NULL != mConnection )
   {
      mConnection->disconnect();
      delete mConnection;
   }
}

}
