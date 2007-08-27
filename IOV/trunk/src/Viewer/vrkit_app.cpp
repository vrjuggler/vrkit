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

#include <stdlib.h>
#include <boost/program_options.hpp>

#include <boost/bind.hpp>

#include <OpenSG/OSGNode.h>
#include <OpenSG/OSGTransform.h>

#include <OpenSG/OSGMatrix.h>
#include <OpenSG/OSGMaterialPool.h>
#include <OpenSG/OSGSimpleGeometry.h>
#include <OpenSG/OSGSimpleAttachments.h>
#include <OpenSG/OSGTransform.h>
#include <OpenSG/OSGDirectionalLight.h>
#include <OpenSG/OSGSceneFileHandler.h>

#include <gmtl/Xforms.h>
#include <gmtl/Vec.h>
#include <gmtl/Generate.h>
#include <gmtl/Coord.h>
#include <gmtl/Matrix.h>
#include <gmtl/MatrixOps.h>
#include <gmtl/External/OpenSGConvert.h>

#include <vpr/vpr.h>
#include <vpr/System.h>
#include <jccl/Config/ConfigElement.h>

#include <vrj/vrjParam.h>
#include <vrj/Kernel/Kernel.h>

#if __VJ_version < 2003011
#  include <vrj/Draw/OGL/GlWindow.h>
#else
#  include <vrj/Draw/OpenGL/Window.h>
#endif

#include <vrj/Kernel/User.h>

#include <vrkit/Viewer.h>
#include <vrkit/scenedata/EventData.h>
#include <vrkit/User.h>
#include <vrkit/Scene.h>
#include <vrkit/DynamicSceneObjectTransform.h>
#include <vrkit/StaticSceneObject.h>
#include <vrkit/WandInterface.h>
#include <vrkit/Exception.h>
#include <vrkit/util/BasicHighlighter.h>
#include <vrkit/util/EventSoundPlayer.h>

#include <vrkit/widget/MaterialChooser.h>
#include <vrkit/scenedata/WidgetData.h>

#include <vrkit/Status.h>

// Video includes.
#include <vrkit/video/Recorder.h>
#include <vrkit/video/Encoder.h>


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
      GLenum errCode = 0;
      if ((errCode = glGetError()) != GL_NO_ERROR)
      {
         const GLubyte *errString = gluErrorString(errCode);
         FWARNING(("%s OpenGL Error: %s!\n", where, errString));
      }

      return errCode == GL_NO_ERROR;
   }
}

template<typename T>
boost::shared_ptr<T> makeSceneObject(OSG::TransformNodePtr modelXform)
{
   return T::create()->init(modelXform);
}

class OpenSgViewer;
typedef boost::shared_ptr<OpenSgViewer> OpenSgViewerPtr;

class OpenSgViewer : public vrkit::Viewer
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

   vrkit::event::ResultType
      objectsMovedSlot(const vrkit::EventData::moved_obj_list_t& objs);

   virtual void init();
   virtual void contextInit();
   virtual void contextPreDraw();
   virtual void draw();
   virtual void preFrame();

   virtual void deallocate();

   // Configuration settings

   void setFilename(std::string filename)
   {
      mFileName = filename;
   }

   void setVideoFilename(const std::string& filename)
   {
      mVideoFileName = filename;
      // If the user passed in a filename they want to record!
      mUseVidRec = true;
   }

   void setVideoCodec(const std::string& codec)
   {
      mVideoCodec = codec;
   }

protected:
   void initGl();

   OpenSgViewer()
      : vrkit::Viewer()
      , mEnableGrab(true)
      , mSceneObjTypeName("StaticSceneObject")
      , mMatChooserWidth(1.0f)  // 1 foot wide
      , mMatChooserHeight(1.5f) // 1.5 feet tall
      , mUseVidRec(false)
      , mVideoCodec("mpeg4")
   {
      /* Do nothing. */ ;
   }

   static std::string getElementType()
   {
      return std::string("vrkit_app");
   }

   void configure(jccl::ConfigElementPtr cfgElt);

protected:
   std::string                      mFileName;
   vrkit::util::BasicHighlighterPtr mHighlighter;
   vrkit::util::EventSoundPlayerPtr mSoundPlayer;

   /** @name Scene Object Handling */
   //@{
   bool        mEnableGrab;
   std::string mSceneObjTypeName;
   //@}

   /** @name Material Chooser Properties */
   //@{
   float                             mMatChooserWidth;
   float                             mMatChooserHeight;
   gmtl::Coord3fXYZ                  mMatChooserCoord;
   vrkit::widget::MaterialChooserPtr mMaterialChooser; /**< The chooser we are using. */
   OSG::MaterialPoolPtr              mMaterialPool;
   //@}

   bool                         mUseVidRec;
   vrkit::video::RecorderPtr    mVideoRecorder;
   std::string                  mVideoFileName;
   std::string                  mVideoCodec;
};

void OpenSgViewer::contextInit()
{
   vrkit::Viewer::contextInit();

   if(mUseVidRec)
   {
      // Context specific data. Should be one copy per context
      context_data* c_data = &(*mContextData);
      mVideoRecorder->contextInit(c_data->mWin);
   }

   initGl();
}

void OpenSgViewer::preFrame()
{
   // Call up to get navigation and plugin updates.
   vrkit::Viewer::preFrame();

   mMaterialChooser->update();

   vrkit::WandInterfacePtr wand_if =
      getUser()->getInterfaceTrader().getWandInterface();
   gadget::DigitalInterface& save_btn(wand_if->getButton(5));

   if ( save_btn->getData() == gadget::Digital::TOGGLE_ON )
   {
      OSG::GroupNodePtr root = getSceneObj()->getSceneRoot();
      OSG::SceneFileHandler::the().write(root, "scene.osb");
   }
}

void OpenSgViewer::draw()
{
   vrkit::Viewer::draw();
}

void OpenSgViewer::contextPreDraw()
{
   vrkit::Viewer::contextPreDraw();

   if(mUseVidRec)
   {
      context_data* c_data = &(*mContextData);

      vrkit::UserPtr vrkit_user = getUser();

      OSG::Matrix4f head_trans;
      gmtl::set(head_trans,
                vrkit_user->getHeadProxy()->getData(getDrawScaleFactor()));

      c_data->mRenderAction->setWindow(c_data->mWin.getCPtr());

      mVideoRecorder->render(c_data->mRenderAction, head_trans);
   }
}

void OpenSgViewer::deallocate()
{
   vrkit::Viewer::deallocate();

   mMaterialChooser = vrkit::widget::MaterialChooserPtr();
   mMaterialPool    = OSG::NullFC;
   mHighlighter     = vrkit::util::BasicHighlighterPtr();
   mSoundPlayer     = vrkit::util::EventSoundPlayerPtr();
   if(mUseVidRec)
   {
      mVideoRecorder->endRecording();
      mVideoRecorder = vrkit::video::RecorderPtr();
   }
}

vrkit::event::ResultType OpenSgViewer::
objectsMovedSlot(const vrkit::EventData::moved_obj_list_t&)
{
   //std::cout << "MOVED" << std::endl;
   //return vrkit::event::DONE;
   return vrkit::event::CONTINUE;
}

void OpenSgViewer::init()
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

   vrkit::EventDataPtr event_data =
      getSceneObj()->getSceneData<vrkit::EventData>();
   event_data->objectsMoved.connect(
      0, boost::bind(&OpenSgViewer::objectsMovedSlot, this, _1)
   );

   vprASSERT(getSceneObj().get() != NULL);      // We should have valid scene

   vrkit::ScenePtr scene = getSceneObj();

   mMaterialChooser = vrkit::widget::MaterialChooser::create();

   // Initialize panel
   const float feet_to_app_units(0.3048f * getDrawScaleFactor());
   mMaterialChooser->init(getDrawScaleFactor());
   mMaterialChooser->setWidthHeight(mMatChooserWidth * feet_to_app_units,
                                    mMatChooserWidth * feet_to_app_units);

   // Set at T*R
   OSG::Matrix xform_osg;
   gmtl::set(xform_osg, gmtl::make<gmtl::Matrix44f>(mMatChooserCoord));
   mMaterialChooser->moveTo(xform_osg);

   // Add widget to scene.
   vrkit::WidgetDataPtr widget_data = scene->getSceneData<vrkit::WidgetData>();
   widget_data->addWidget(mMaterialChooser);
   addObject(mMaterialChooser);

   OSG::RefPtr<OSG::NodePtr> model_root;

   // Load the model to use
   if (mFileName.empty())
   {
      std::cout << "Loading fake geometry." << std::endl;
      model_root = OSG::makeTorus(0.75, 2.1, 21, 21);
   }
   else
   {
      std::cout << "Loading scene: " << mFileName << std::endl;
      model_root =
         OSG::SceneFileHandler::the().read((OSG::Char8*) (mFileName.c_str()));
   }

   mMaterialPool = OSG::MaterialPool::create();
   mMaterialPool->add(model_root);
   mMaterialChooser->setMaterialPool(mMaterialPool);

   OSG::TransformPtr model_xform_core = OSG::Transform::create();
   OSG::beginEditCP(model_xform_core, OSG::Transform::MatrixFieldMask);
      OSG::Matrix model_pos;
      model_xform_core->setMatrix(model_pos);
   OSG::endEditCP(model_xform_core, OSG::Transform::MatrixFieldMask);

   OSG::TransformNodePtr model_xform = OSG::TransformNodePtr(model_xform_core);
   OSG::setName(model_xform.node(), "Model Xform");

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
   light_pos.setTransform(OSG::Vec3f(2.0f, 5.0f, 4.0f));

   OSG::beginEditCP(light_beacon_core, OSG::Transform::MatrixFieldMask);
      light_beacon_core->setMatrix(light_pos);
   OSG::endEditCP(light_beacon_core, OSG::Transform::MatrixFieldMask);

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
      light_core->setBeacon    (light_beacon);
   OSG::endEditCP(light_core);

   // --- Setup Scene -- //
   // add the loaded scene to the light node, so that it is lit by the light
   OSG::beginEditCP(light_node);
      light_node->addChild(model_xform);
   OSG::endEditCP(light_node);

   // create the root part of the scene
   OSG::TransformNodePtr scene_transform_root = scene->getTransformRoot();

   // Set up the root node
   OSG::beginEditCP(scene_transform_root.node());
      scene_transform_root.node()->addChild(light_node);
   OSG::endEditCP(scene_transform_root.node());

   if ( mEnableGrab )
   {
      vrkit::SceneObjectPtr model_obj;

      if ( mSceneObjTypeName == "StaticSceneObject" )
      {
         model_obj = makeSceneObject<vrkit::StaticSceneObject>(model_xform);
      }
      else if ( mSceneObjTypeName == "DynamicSceneObject" )
      {
         model_obj =
            makeSceneObject<vrkit::DynamicSceneObjectTransform>(model_xform);
      }
      else
      {
         VRKIT_STATUS << "ERROR: Invalid scene object type name '"
                      << mSceneObjTypeName << "'" << std::endl
                      << "Falling back on StaticSceneObject" << std::endl;
         model_obj = makeSceneObject<vrkit::StaticSceneObject>(model_xform);
      }

      // Register object for intersection.
      addObject(model_obj);
   }

   mVideoRecorder = vrkit::video::RecorderPtr(); // Init when not using vidcam

   if(mUseVidRec)
   {
      mVideoRecorder = vrkit::video::Recorder::create()->init();
      mVideoRecorder->setFilename(mVideoFileName);
      //mVideoRecorder->setFrameSize(1024, 768);
      //mVideoRecorder->setStereo(true);
      vrkit::video::Encoder::container_format_list_t formats =
         mVideoRecorder->getAvailableFormats();

#if 1
      // Debug code for printing out all choices. Should get relocated to Encoder?
      std::cout << "Encoding Format Choices" << std::endl;
      std::cout << formats.size() << " choices.." << std::endl;
      std::cout << "-------------------------------------" << std::endl;
      typedef vrkit::video::Encoder::container_format_list_t::const_iterator
         iter_type;
      for ( iter_type i = formats.begin(); i != formats.end(); ++i )
      {
	 std::cout << (*i).mFormatLongName << std::endl;
	 std::cout << "  id         : " << (*i).mFormatName << std::endl;
	 std::cout << "  encoder    : " << (*i).mEncoderName << std::endl;
	 std::cout << "  codecs     :" << std::endl;
	 if( 0 == (*i).mCodecList.size() )
	 {
	    std::cout << "               default" << std::endl;
	 }
	 else
	 {
            typedef vrkit::video::Encoder::codec_list_t::const_iterator
               codec_iter_type;
            for ( codec_iter_type j = (*i).mCodecList.begin();
                  j != (*i).mCodecList.end();
                  ++j )
	    {
	       std::cout << "               " << (*j) << std::endl;
	    }
	 }
	 std::cout << "  extensions :" << std::endl;
         typedef std::vector<std::string>::const_iterator ext_iter_type;
         for ( ext_iter_type j = (*i).mFileExtensions.begin();
               j != (*i).mFileExtensions.end();
               ++j )
	 {
	    std::cout << "               " << (*j) << std::endl;
	 }
      std::cout << "-------------------------------------" << std::endl;
      }
#endif

      vrkit::video::EncoderManager::video_encoder_format_t config;
      //config.mEncoderName = "FFmpegEncoder";
      //config.mContainerFormat = "avi";
      //config.mCodec = "mpeg4";
      // XXX: How to pick defaults?
      config.mEncoderName = formats[0].mEncoderName;
      config.mContainerFormat = formats[0].mFormatName;
      config.mCodec = "";
      mVideoRecorder->setFormat(config);

      mVideoRecorder->startRecording();

      OSG::NodePtr frame_root = mVideoRecorder->getFrame();

      // Create the test plane to put the texture on.
      OSG::NodePtr plane_root = mVideoRecorder->getDebugPlane();

      OSG::GroupNodePtr decorator_root = scene->getDecoratorRoot();
      OSG::beginEditCP(decorator_root);
	 decorator_root.node()->addChild(plane_root);
	 decorator_root.node()->addChild(frame_root);
      OSG::endEditCP(decorator_root);

      // Ensure that this is called after anythin that effects getScene().
      // VR Juggler normally calls this each frame before rendering.
      mVideoRecorder->setSceneRoot(getScene());
   }
}

void OpenSgViewer::initGl()
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

void OpenSgViewer::configure(jccl::ConfigElementPtr cfgElt)
{
   const unsigned int req_cfg_ver(1);

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
   const std::string scene_obj_type_prop("scene_object_type");
   const std::string use_mchooser_prop("use_material_chooser");
   const std::string initial_size_prop("chooser_initial_size");
   const std::string initial_pos_prop("chooser_initial_position");
   const std::string initial_rot_prop("chooser_initial_rotation");

   mEnableGrab       = cfgElt->getProperty<bool>(enable_grab_prop);
   mSceneObjTypeName = cfgElt->getProperty<std::string>(scene_obj_type_prop);

   const float width  = cfgElt->getProperty<float>(initial_size_prop, 0);
   const float height = cfgElt->getProperty<float>(initial_size_prop, 1);

   if ( width > 0.0f )
   {
      mMatChooserWidth = width;
   }
   else
   {
      VRKIT_STATUS << "ERROR: Invalid width for material chooser " << width
                   << std::endl;
   }

   if ( height > 0.0f )
   {
      mMatChooserHeight = height;
   }
   else
   {
      VRKIT_STATUS << "ERROR: Invalid height for material chooser " << height
                   << std::endl;
   }

   const float feet_to_app_units(0.3048f * getDrawScaleFactor());

   const float xt = cfgElt->getProperty<float>(initial_pos_prop, 0);
   const float yt = cfgElt->getProperty<float>(initial_pos_prop, 1);
   const float zt = cfgElt->getProperty<float>(initial_pos_prop, 2);

   const float xr = cfgElt->getProperty<float>(initial_rot_prop, 0);
   const float yr = cfgElt->getProperty<float>(initial_rot_prop, 1);
   const float zr = cfgElt->getProperty<float>(initial_rot_prop, 2);

   mMatChooserCoord.pos().set(xt * feet_to_app_units,
                              yt * feet_to_app_units,
                              zt * feet_to_app_units);
   mMatChooserCoord.rot().set(gmtl::Math::deg2Rad(xr),
                              gmtl::Math::deg2Rad(yr),
                              gmtl::Math::deg2Rad(zr));
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
         ("video-file,v",
          po::value<std::string>()->composing(),
          "File to save video to.")
         ("video-codec,c",
          po::value<std::string>()->composing(),
          "Codec to encode video with.")
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

      for ( std::vector<std::string>::iterator i = jdef_dirs.begin();
            i != jdef_dirs.end();
            ++i )
      {
         VRKIT_STATUS << *i << " ";
      }

      VRKIT_STATUS << std::endl;

      if ( ! jdef_dirs.empty() )
      {
         VRKIT_STATUS << "Scanning for definitions." << std::endl;

         if ( ! jdef_dirs.empty() )
         {
            for ( std::vector<std::string>::iterator i = jdef_dirs.begin();
                  i != jdef_dirs.end();
                  ++i )
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

      if (vm.count("video-file") != 0)
      {
	 std::cout << "Setting video file name!" << std::endl;
	 std::string video_file = vm["video-file"].as<std::string>();
	 app->setVideoFilename(video_file);
      }

      if (vm.count("video-codec") != 0)
      {
	 std::cout << "Setting video codec!" << std::endl;
	 std::string video_codec = vm["video-codec"].as<std::string>();
	 app->setVideoCodec(video_codec);
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
