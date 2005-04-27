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

#include <OpenSG/VRJ/Viewer/User.h>

class OpenSgViewer;
typedef boost::shared_ptr<OpenSgViewer> OpenSgViewerPtr;


class OpenSgViewer : public inf::Viewer
{
public:
   static OpenSgViewerPtr create()
   {
      OpenSgViewerPtr new_viewer(new OpenSgViewer);
      return new_viewer;
   }

   virtual ~OpenSgViewer()
   {;}

   // Callback methods

   virtual void init();
   virtual void contextInit();
   virtual void preFrame();
   virtual void postFrame()
   {;}


   // Configuration settings

   void setFilename(std::string filename)
   {
      mFileName = filename;
   }

protected:
   void initGl();

   OpenSgViewer()
      : inf::Viewer()
   {;}

protected:
   std::string    mFileName;
};


void OpenSgViewer::contextInit()
{
   inf::Viewer::contextInit();
   initGl();
}

void OpenSgViewer::preFrame()
{
   // Call up to get navigation and plugin updates.
   inf::Viewer::preFrame();
   // Do nothing
}


void OpenSgViewer::init()
{
   inf::Viewer::init();

   vprASSERT(getSceneObj().get() != NULL);      // We should have valid scene

   OSG::RefPtr<OSG::NodePtr> model_root;

   // Load the model to use
   if (mFileName.empty())
   {
      std::cout << "Loading fake geometry." << std::endl;
      model_root = osg::makeTorus(0.75, 2.1, 21, 21);
   }
   else
   {
      std::cout << "Loading scene: " << mFileName << std::endl;
      model_root =
         osg::SceneFileHandler::the().read((osg::Char8*)(mFileName.c_str()));
   }

   // --- Light setup --- //
   // - Add directional light for scene
   // - Create a beacon for it and connect to that beacon
   OSG::NodePtr  light_node;
   OSG::NodePtr  light_beacon;
   light_node   = OSG::Node::create();
   light_beacon = OSG::Node::create();
   OSG::DirectionalLightPtr light_core = OSG::DirectionalLight::create();
   OSG::TransformPtr light_beacon_core = OSG::Transform::create();

   // Setup light beacon
   OSG::Matrix light_pos;
   light_pos.setTransform(osg::Vec3f(2.0f, 5.0f, 4.0f));

   OSG::beginEditCP(light_beacon_core, osg::Transform::MatrixFieldMask);
      light_beacon_core->setMatrix(light_pos);
   OSG::endEditCP(light_beacon_core, osg::Transform::MatrixFieldMask);

   OSG::beginEditCP(light_beacon);
      light_beacon->setCore(light_beacon_core);
   OSG::endEditCP(light_beacon);

   // Setup light node
   OSG::addRefCP(light_node);
   OSG::beginEditCP(light_node);
      light_node->setCore(light_core);
      light_node->addChild(light_beacon);
   OSG::endEditCP(light_node);

   OSG::beginEditCP(light_core);
      light_core->setAmbient   (0.9, 0.8, 0.8, 1);
      light_core->setDiffuse   (0.6, 0.6, 0.6, 1);
      light_core->setSpecular  (1, 1, 1, 1);
      light_core->setDirection (0, 0, 1);
      light_core->setBeacon    (light_node);
   OSG::endEditCP(light_core);

   // --- Setup Scene -- //
   // add the loaded scene to the light node, so that it is lit by the light
   OSG::beginEditCP(light_node);
      light_node->addChild(model_root);
   OSG::endEditCP(light_node);

   // create the root part of the scene
   inf::CoredTransformPtr scene_transform_root = getSceneObj()->getTransformRoot();

   // Set up the root node
   osg::beginEditCP(scene_transform_root.node());
      scene_transform_root.node()->addChild(light_node);
   osg::endEditCP(scene_transform_root.node());
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
   OpenSgViewerPtr app = OpenSgViewer::create();      // Create the app object

   for(int i=1; i<argc; ++i)
   {
      kernel->loadConfigFile(argv[i]);      // Configure the kernel
   }

   kernel->start();                         // Start the kernel thread
   kernel->setApplication(app.get());             // Give application to kernel
   kernel->waitForKernelStop();             // Block until kernel stops

   return 0;
}


