#include <vrj/Kernel/Kernel.h>
#include <vrj/Draw/OpenSG/OpenSGApp.h>

#include <OpenSG/OSGNode.h>
#include <OpenSG/OSGTransform.h>


#include <OpenSG/OSGMatrix.h>
#include <OpenSG/OSGSimpleGeometry.h>
#include <OpenSG/OSGDirectionalLight.h>
#include <OpenSG/OSGSceneFileHandler.h>

#include <gmtl/Xforms.h>
#include <gmtl/Vec.h>
#include <gmtl/Generate.h>
#include <gmtl/Coord.h>
#include <gmtl/Matrix.h>
#include <gmtl/MatrixOps.h>

#include <OpenSG/VRJ/Viewer/Viewer.h>


class OpenSgViewer : public vrj::OpenSGApp
{
public:
   OpenSgViewer(vrj::Kernel* kern)
      : vrj::OpenSGApp(kern)
   {;}
   virtual ~OpenSgViewer()
   {;}
   virtual void init();
   virtual void contextInit();
   virtual void preFrame();
   virtual void postFrame()
   {;}

   virtual void initScene();
   virtual OSG::NodePtr getScene()
   {
      return mSceneRoot;
   }

   void setFilename(std::string filename)
   {
      mFileName = filename;
   }

protected:
   void initGl();

protected:
   std::string    mFileName;

   OSG::NodePtr        mSceneRoot;
   OSG::TransformPtr   mSceneTransform;
   OSG::NodePtr        mModelRoot;

   OSG::NodePtr  mLightNode;
   OSG::NodePtr  mLightBeacon;
};


void OpenSgViewer::init()
{
   vrj::OpenSGApp::init();
}

void OpenSgViewer::contextInit()
{
   vrj::OpenSGApp::contextInit();
   initGl();
}

void OpenSgViewer::preFrame()
{
   // Do nothing
}


void OpenSgViewer::initScene()
{
   // Load the model to use
   if (mFileName.empty())
   {
      std::cout << "Loading fake geometry." << std::endl;
      mModelRoot = osg::makeTorus(0.75, 2.1, 21, 21);
   }
   else
   {
      std::cout << "Loading scene: " << mFileName << std::endl;
      mModelRoot =
         osg::SceneFileHandler::the().read((osg::Char8*)(mFileName.c_str()));
   }

   // --- Light setup --- //
   // - Add directional light for scene
   // - Create a beacon for it and connect to that beacon
   mLightNode   = osg::Node::create();
   mLightBeacon = osg::Node::create();
   osg::DirectionalLightPtr light_core = osg::DirectionalLight::create();
   osg::TransformPtr light_beacon_core = osg::Transform::create();

   // Setup light beacon
   osg::Matrix light_pos;
   light_pos.setTransform(osg::Vec3f(2.0f, 5.0f, 4.0f));

   osg::beginEditCP(light_beacon_core, osg::Transform::MatrixFieldMask);
      light_beacon_core->setMatrix(light_pos);
   osg::endEditCP(light_beacon_core, osg::Transform::MatrixFieldMask);

   osg::beginEditCP(mLightBeacon);
      mLightBeacon->setCore(light_beacon_core);
   osg::endEditCP(mLightBeacon);

   // Setup light node
   osg::addRefCP(mLightNode);
   osg::beginEditCP(mLightNode);
      mLightNode->setCore(light_core);
      mLightNode->addChild(mLightBeacon);
   osg::endEditCP(mLightNode);

   osg::beginEditCP(light_core);
      light_core->setAmbient   (0.9, 0.8, 0.8, 1);
      light_core->setDiffuse   (0.6, 0.6, 0.6, 1);
      light_core->setSpecular  (1, 1, 1, 1);
      light_core->setDirection (0, 0, 1);
      light_core->setBeacon    (mLightNode);
   osg::endEditCP(light_core);

   // --- Setup Scene -- //
   // add the loaded scene to the light node, so that it is lit by the light
   osg::addRefCP(mModelRoot);
   osg::beginEditCP(mLightNode);
      mLightNode->addChild(mModelRoot);
   osg::endEditCP(mLightNode);

   // create the root part of the scene
   mSceneRoot = osg::Node::create();
   mSceneTransform = osg::Transform::create();

   // Set up the root node
   osg::beginEditCP(mSceneRoot);
      mSceneRoot->setCore(mSceneTransform);
      mSceneRoot->addChild(mLightNode);
   osg::endEditCP(mSceneRoot);
}



void OpenSgViewer::initGl()
{
   glEnable(GL_NORMALIZE);
}



int main(int argc, char* argv[])
{
   if(argc < 2)
   {
      std::cerr << "Not enough arguments:  app config-file(s)" << std::endl;
   }

   vrj::Kernel* kernel = vrj::Kernel::instance();     // Get the kernel
   OpenSgViewer* app   = new OpenSgViewer(kernel);    // Create the app object

   for(int i=1; i<argc; ++i)
   {
      kernel->loadConfigFile(argv[i]);      // Configure the kernel
   }

   kernel->start();                         // Start the kernel thread
   kernel->setApplication(app);             // Give application to kernel
   kernel->waitForKernelStop();             // Block until kernel stops

   return 0;
}


