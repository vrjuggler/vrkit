// Copyright (C) Infiscape Corporation 2005-2006

#include <stdlib.h>
#include <boost/program_options.hpp>

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

#include <vpr/vpr.h>
#include <vpr/System.h>
#include <vrj/Kernel/Kernel.h>

#include <IOV/Viewer.h>
#include <IOV/User.h>
#include <IOV/Scene.h>
#include <IOV/GrabData.h>

#include <IOV/Status.h>


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
         osg::SceneFileHandler::the()->read((osg::Char8*)(mFileName.c_str()));
   }

   OSG::TransformPtr model_xform_core = OSG::Transform::create();
   OSG::beginEditCP(model_xform_core, OSG::Transform::MatrixFieldMask);
      OSG::Matrix model_pos;
      model_xform_core->setMatrix(model_pos);
   OSG::endEditCP(model_xform_core, OSG::Transform::MatrixFieldMask);

   OSG::TransformNodePtr model_xform = OSG::TransformNodePtr(model_xform_core);

   OSG::beginEditCP(model_xform);
      model_xform.node()->addChild(model_root);
   OSG::endEditCP(model_xform);

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
   OSG::addRef(light_node);
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
      light_node->addChild(model_xform);
   OSG::endEditCP(light_node);

   // create the root part of the scene
   inf::ScenePtr scene = getSceneObj();
   OSG::TransformNodePtr scene_transform_root = scene->getTransformRoot();

   // Set up the root node
   osg::beginEditCP(scene_transform_root.node());
      scene_transform_root.node()->addChild(light_node);
   osg::endEditCP(scene_transform_root.node());

   inf::GrabDataPtr grab_data = scene->getSceneData<inf::GrabData>();
   grab_data->addObject(model_xform);
}

void OpenSgViewer::initGl()
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

namespace po = boost::program_options;

int main(int argc, char* argv[])
{
   const int EXIT_ERR_MISSING_JCONF(1);
   const int EXIT_ERR_MISSING_APP_CONF(2);
   const int EXIT_ERR_EXCEPTION(-1);

   try
   {
      po::options_description generic("General options");
      generic.add_options()
         ("help", "produce help message")
         ;

      std::string jdef_dir;

      po::options_description config("Configuration");
      config.add_options()
         ("jconf,j", po::value< std::vector<std::string> >()->composing(),
          "VR Juggler config file")
         ("app,a", po::value< std::vector<std::string> >()->composing(),
          "Viewer configuration file")
         ("defs,d", po::value<std::string>(&jdef_dir),
          "Path to custom config definition (.jdef) files")
         ("file,f",
          po::value<std::string>()->default_value("data/scenes/test_scene.osb"),
          "File to load in scene")
      ;

      po::options_description cmdline_options;
      cmdline_options.add(generic).add(config);

      po::options_description config_file_options;
      config_file_options.add(config);

      po::variables_map vm;
      store(po::command_line_parser(argc, argv).options(cmdline_options).run(),
            vm);

      std::ifstream cfg_file("viewer.cfg");
      store(parse_config_file(cfg_file, config_file_options), vm);
      notify(vm);

      if ( vm.count("help") > 0 )
      {
         std::cout << cmdline_options << std::endl;
         return EXIT_SUCCESS;
      }

      vrj::Kernel* kernel = vrj::Kernel::instance();  // Get the kernel
      OpenSgViewerPtr app = OpenSgViewer::create();   // Create the app object

      if ( vm.count("defs") == 0 )
      {
         std::string iov_base_dir;
         vpr::System::getenv("IOV_BASE_DIR", iov_base_dir);

         if ( iov_base_dir.empty() )
         {
            std::cerr << "WARNING: Environment variable IOV_BASE_DIR is not\n"
                      << "         set or is set to an empty value.  Expect\n"
                      << "         problems later." << std::endl;
         }
         else
         {
            jdef_dir = iov_base_dir + std::string("/share/IOV/definitions");
         }
      }
      IOV_STATUS << "Using jdef path: " << jdef_dir << std::endl;

      if ( ! jdef_dir.empty() )
      {
         IOV_STATUS << "Scanning for definitions." << std::endl;
         kernel->scanForConfigDefinitions(jdef_dir);
      }

      if ( vm.count("jconf") == 0 )
      {
         std::cout << "No VR Juggler configuration files given!" << std::endl;
         return EXIT_ERR_MISSING_JCONF;
      }
      else
      {
         std::vector<std::string> jconfs =
            vm["jconf"].as< std::vector<std::string> >();

         std::vector<std::string>::iterator i;
         for ( i = jconfs.begin(); i != jconfs.end(); ++i )
         {
            kernel->loadConfigFile(*i);
         }
      }

      if ( vm.count("app") == 0 )
      {
         std::cout << "No application configuration file given!" << std::endl;
         return EXIT_ERR_MISSING_APP_CONF;
      }
      else
      {
         // Load named application configuration file(s).
         std::vector<std::string> jconfs =
            vm["app"].as< std::vector<std::string> >();

         std::vector<std::string>::iterator i;
         for ( i = jconfs.begin(); i != jconfs.end(); ++i )
         {
            app->getConfiguration().loadConfigEltFile(*i);
         }
      }

      if (vm.count("file") != 0)
      {
         std::string filename = vm["file"].as<std::string>();
         app->setFilename(filename);
      }

      IOV_STATUS << "Starting the kernel." << std::endl;

      kernel->start();                         // Start the kernel thread
      kernel->setApplication(app.get());       // Give application to kernel
      kernel->waitForKernelStop();             // Block until kernel stops
   }
   catch (std::exception& ex)
   {
      std::cout << ex.what() << std::endl;
      return EXIT_ERR_EXCEPTION;
   }

   return EXIT_SUCCESS;
}
