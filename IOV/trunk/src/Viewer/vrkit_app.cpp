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

#include <cstdlib>
#include <vector>
#include <boost/program_options.hpp>
#include <boost/bind.hpp>

#include <OpenSG/OSGNode.h>
#include <OpenSG/OSGTransform.h>
#include <OpenSG/OSGMatrix.h>
#include <OpenSG/OSGSimpleGeometry.h>
#include <OpenSG/OSGDirectionalLight.h>
#include <OpenSG/OSGSceneFileHandler.h>

#include <vpr/vpr.h>
#include <vpr/System.h>
#include <jccl/Config/ConfigElement.h>
#include <vrj/vrjParam.h>
#include <vrj/Kernel/Kernel.h>
#include <vrj/Kernel/User.h>

#include <vrkit/Viewer.h>
#include <vrkit/User.h>
#include <vrkit/Scene.h>
#include <vrkit/Status.h>
#include <vrkit/DynamicSceneObject.h>
#include <vrkit/WandInterface.h>
#include <vrkit/Exception.h>
#include <vrkit/scenedata/EventData.h>
#include <vrkit/scenedata/MaterialPoolData.h>
#include <vrkit/util/BasicHighlighter.h>
#include <vrkit/util/EventSoundPlayer.h>
#include <vrkit/util/CoreTypePredicate.h>


#if __VJ_version < 2003002
namespace vrj
{

typedef class User* UserPtr;

}
#endif

namespace
{

bool checkGLError(const char* where)
{
   const GLenum err_code(glGetError());
   if ( err_code != GL_NO_ERROR )
   {
      FWARNING(("%s OpenGL Error: %s!\n", where,
                gluErrorString(err_code)));
   }

   return err_code == GL_NO_ERROR;
}

}

class VrkitApp;
typedef boost::shared_ptr<VrkitApp> VrkitAppPtr;

class VrkitApp : public vrkit::Viewer
{
private:
   VrkitApp()
      : vrkit::Viewer()
      , mEnableGrab(true)
   {
      /* Do nothing. */ ;
   }

public:
   static VrkitAppPtr create()
   {
      return VrkitAppPtr(new VrkitApp());
   }

   virtual ~VrkitApp()
   {
      /* Do nothing. */ ;
   }

   // Callback methods

   virtual void init();
   virtual void contextInit();
   virtual void preFrame();

   virtual void deallocate();

   // Configuration settings

   void setFilename(const std::string& filename)
   {
      mFileName = filename;
   }

private:
   void initGl();

   static std::string getElementType()
   {
      return std::string("vrkit_app");
   }

   void configure(jccl::ConfigElementPtr cfgElt);

   vrkit::event::ResultType objectsMovedSlot();

   std::string                      mFileName;
   vrkit::util::BasicHighlighterPtr mHighlighter;
   vrkit::util::EventSoundPlayerPtr mSoundPlayer;

   /** @name Scene Object Handling */
   //@{
   bool mEnableGrab;
   std::vector<OSG::FieldContainerType*> mCoreTypes;
   //@}
};

void VrkitApp::contextInit()
{
   vrkit::Viewer::contextInit();

   initGl();
}

void VrkitApp::preFrame()
{
   // Call up to get navigation and plugin updates.
   vrkit::Viewer::preFrame();

   vrkit::WandInterfacePtr wand_if =
      getUser()->getInterfaceTrader().getWandInterface();
   gadget::DigitalInterface& save_btn(wand_if->getButton(5));

   if ( save_btn->getData() == gadget::Digital::TOGGLE_ON )
   {
      OSG::GroupNodePtr root = getSceneObj()->getSceneRoot();
      OSG::SceneFileHandler::the().write(root, "scene.osb");
   }
}

void VrkitApp::deallocate()
{
   vrkit::Viewer::deallocate();

   mHighlighter = vrkit::util::BasicHighlighterPtr();
   mSoundPlayer = vrkit::util::EventSoundPlayerPtr();
}

void VrkitApp::init()
{
   vrkit::Viewer::init();

   jccl::ConfigElementPtr app_cfg =
      getConfiguration().getConfigElement(getElementType());

   if ( app_cfg )
   {
      try
      {
         configure(app_cfg);
      }
      catch (vrkit::Exception& ex)
      {
         std::cerr << ex.what() << std::endl;
      }
   }

   vrkit::ViewerPtr myself(shared_from_this());
   mHighlighter = vrkit::util::BasicHighlighter::create()->init(myself);
   mSoundPlayer = vrkit::util::EventSoundPlayer::create()->init(myself);

   vrkit::ScenePtr scene = getSceneObj();

   vrkit::EventDataPtr event_data = scene->getSceneData<vrkit::EventData>();

   // The vrkit::EventData::objectsMoved signal provides a vector of
   // vrkit::SceneObjectPtr objects that are the objects being moved. We
   // ignore that vector by not passing _1 to boost::bind() here, but
   // normally, a slot would want to know about those objects.
   event_data->objectsMoved.connect(
      0, boost::bind(&VrkitApp::objectsMovedSlot, this)
   );

   OSG::NodeRefPtr model_root;

   // Load the model to use
   if ( mFileName.empty() )
   {
      std::cout << "Loading fake geometry." << std::endl;
      model_root = OSG::makeTorus(0.75f, 2.1f, 21, 21);
   }
   else
   {
      std::cout << "Loading scene: " << mFileName << std::endl;
      model_root = OSG::SceneFileHandler::the().read(mFileName.c_str());

      if ( OSG::NullFC == model_root )
      {
         std::cerr << "FATAL ERROR: Failed to load " << mFileName << "!\n";
         std::exit(-2);
      }
   }

   // Make the materials of the loaded model available in the material pool.
   // This is only here so that the Material Chooser Plug-in can have
   // something to work with if it is configured to be used.
   // XXX: This is a little clunky, and it might be better if this were done
   // by vrkit::Viewer::addObject().
   vrkit::MaterialPoolDataPtr mat_pool =
      scene->getSceneData<vrkit::MaterialPoolData>();
   mat_pool->getMaterialPool()->add(model_root);

   // --- Light setup --- //
   // - Add directional light for scene
   // - Create a beacon for it and connect to that beacon
   OSG::DirectionalLightNodePtr light_node(OSG::DirectionalLight::create());
   OSG::TransformNodePtr light_beacon(OSG::Transform::create());

   // Set up light beacon.
   OSG::Matrix light_pos;
   light_pos.setTransform(OSG::Vec3f(2.0f, 5.0f, 4.0f));

   OSG::beginEditCP(light_beacon, OSG::Transform::MatrixFieldMask);
      light_beacon->setMatrix(light_pos);
   OSG::endEditCP(light_beacon, OSG::Transform::MatrixFieldMask);

   // Set up light node.
   OSG::beginEditCP(light_node.node(), OSG::Node::ChildrenFieldMask);
      light_node.node()->addChild(light_beacon);
   OSG::endEditCP(light_node.node(), OSG::Node::ChildrenFieldMask);

   OSG::beginEditCP(light_node);
      light_node->setAmbient(0.9f, 0.8f, 0.8f, 1.0f);
      light_node->setDiffuse(0.6f, 0.6f, 0.6f, 1.0f);
      light_node->setSpecular(1.0f, 1.0f, 1.0f, 1.0f);
      light_node->setDirection(0.0f, 0.0f, 1.0f);
      light_node->setBeacon(light_beacon);
   OSG::endEditCP(light_node);

   // --- Set up Scene -- //
   // add the loaded scene to the light node, so that it is lit by the light
   OSG::beginEditCP(light_node.node(), OSG::Node::ChildrenFieldMask);
      light_node.node()->addChild(model_root);
   OSG::endEditCP(light_node.node(), OSG::Node::ChildrenFieldMask);

   // create the root part of the scene
   OSG::TransformNodePtr scene_transform_root = scene->getTransformRoot();

   // Set up the root node
   OSG::beginEditCP(scene_transform_root.node(), OSG::Node::ChildrenFieldMask);
      scene_transform_root.node()->addChild(light_node);
   OSG::endEditCP(scene_transform_root.node(), OSG::Node::ChildrenFieldMask);

   if ( mEnableGrab )
   {
      // Register object for intersection and grabbing.
      vrkit::util::CoreTypePredicate pred(mCoreTypes);
      addObject(
         vrkit::DynamicSceneObject::create()->init(model_root, pred, true)
      );
   }
}

void VrkitApp::initGl()
{
   //glEnable(GL_NORMALIZE);

#if __VJ_version < 2001025
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

void VrkitApp::configure(jccl::ConfigElementPtr cfgElt)
{
   const unsigned int req_cfg_ver(2);

   if ( cfgElt->getVersion() < req_cfg_ver )
   {
      std::ostringstream msg_stream;
      msg_stream << "WARNING: vrkit viewer application config element '"
                 << cfgElt->getName() << "' is out of date!" << std::endl
                 << "         Current config element version is "
                 << req_cfg_ver << ", but this one is version "
                 << cfgElt->getVersion() << std::endl;
      throw vrkit::Exception(msg_stream.str(), VRKIT_LOCATION);
   }

   const std::string enable_grab_prop("enable_grabbing");
   const std::string core_type_prop("core_type");

   mEnableGrab = cfgElt->getProperty<bool>(enable_grab_prop);

   const unsigned int num_types = cfgElt->getNum(core_type_prop);
   for ( unsigned int i = 0; i < num_types; ++i )
   {
      const std::string type_name = 
         cfgElt->getProperty<std::string>(core_type_prop, i);
      OSG::FieldContainerType* fct =
         OSG::FieldContainerFactory::the()->findType(type_name.c_str());

      if ( NULL != fct )
      {
         mCoreTypes.push_back(fct);
      }
      else
      {
         VRKIT_STATUS << "Skipping unknown type '" << type_name << "'"
                      << std::endl;
      }
   }
}

// This is a demonstration of how a slot for the objectsMoved signal might be
// written.
vrkit::event::ResultType VrkitApp::objectsMovedSlot()
{
   //std::cout << "MOVED" << std::endl;
   //return vrkit::event::DONE;
   return vrkit::event::CONTINUE;
}

int main(int argc, char* argv[])
{
   namespace po = boost::program_options;

   const int EXIT_ERR_MISSING_JCONF(1);
   const int EXIT_ERR_MISSING_APP_CONF(2);
   const int EXIT_ERR_EXCEPTION(-1);

   try
   {
      vrj::Kernel* kernel = vrj::Kernel::instance();  // Get the kernel

      po::options_description generic("General options");
#if __VJ_version >= 2003000
      po::options_description& general_desc = kernel->getGeneralOptions();
      po::options_description& cluster_desc = kernel->getClusterOptions();
      generic.add(general_desc).add(cluster_desc);
#endif

      generic.add_options()
         ("help", "produce help message")
         ;

      std::vector<std::string> jdef_dirs;

      po::options_description config("Configuration");
      config.add_options()
         ("jconf,j", po::value< std::vector<std::string> >()->composing(),
          "VR Juggler config file")
         ("app,a", po::value< std::vector<std::string> >()->composing(),
          "Viewer configuration file")
         ("defs,d", po::value< std::vector<std::string> >(&jdef_dirs),
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

      VrkitAppPtr app = VrkitApp::create();   // Create the app object

#if __VJ_version >= 2003000
      // Intialize the kernel before loading config files.
      kernel->init(vm);
#endif

      if ( vm.count("defs") == 0 )
      {
         std::string vrkit_base_dir;
         vpr::System::getenv("VRKIT_BASE_DIR", vrkit_base_dir);

         if ( vrkit_base_dir.empty() )
         {
            std::cerr << "WARNING: Environment variable VRKIT_BASE_DIR is\n"
                      << "         not set or is set to an empty value.\n"
                      << "         Expect problems later." << std::endl;
            jdef_dirs.push_back(".");
         }
         else
         {
            jdef_dirs.push_back(vrkit_base_dir +
                                   std::string("/share/vrkit/definitions"));
            jdef_dirs.push_back(".");
         }
      }

      VRKIT_STATUS << "Using jdef path: ";

      typedef std::vector<std::string>::iterator iter_type;
      for ( iter_type i = jdef_dirs.begin(); i != jdef_dirs.end(); ++i )
      {
         VRKIT_STATUS << *i << " ";
      }

      VRKIT_STATUS << std::endl;

      if ( ! jdef_dirs.empty() )
      {
         VRKIT_STATUS << "Scanning for definitions." << std::endl;

         if ( ! jdef_dirs.empty() )
         {
            for ( iter_type i = jdef_dirs.begin(); i != jdef_dirs.end(); ++i )
            {
               kernel->scanForConfigDefinitions(*i);
            }
         }
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

      VRKIT_STATUS << "Starting the kernel." << std::endl;

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
