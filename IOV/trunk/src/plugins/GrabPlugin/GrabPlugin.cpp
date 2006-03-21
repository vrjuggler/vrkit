// Copyright (C) Infiscape Corporation 2005-2006

#include <string.h>
#include <algorithm>
#include <boost/bind.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/operations.hpp>

#include <OpenSG/OSGSimpleMaterial.h>

#include <gmtl/Matrix.h>
#include <gmtl/MatrixOps.h>
#include <gmtl/External/OpenSGConvert.h>

#include <vpr/vpr.h>
#include <vpr/System.h>
#include <vpr/Util/FileUtils.h>

#include <IOV/EventData.h>
#include <IOV/Viewer.h>
#include <IOV/PluginCreator.h>
#include <IOV/PluginPtr.h>
#include <IOV/User.h>
#include <IOV/InterfaceTrader.h>
#include <IOV/WandInterface.h>
#include <IOV/ViewPlatform.h>
#include <IOV/Status.h>
#include <IOV/StatusPanel.h>
#include <IOV/StatusPanelPlugin.h>
#include <IOV/Util/Exceptions.h>

#include "GrabPlugin.h"


namespace fs = boost::filesystem;

static inf::PluginCreator<inf::Plugin> sPluginCreator(&inf::GrabPlugin::create,
                                                      "Grab Plug-in");

extern "C"
{

/** @name Plug-in Entry Points */
//@{
IOV_PLUGIN_API(void) getPluginInterfaceVersion(vpr::Uint32& majorVer,
                                               vpr::Uint32& minorVer)
{
   majorVer = INF_PLUGIN_API_MAJOR;
   minorVer = INF_PLUGIN_API_MINOR;
}

IOV_PLUGIN_API(inf::PluginCreator<inf::Plugin>*) getCreator()
{
   return &sPluginCreator;
}
//@}

}

namespace inf
{

void GrabPlugin::init(ViewerPtr viewer)
{
   mEventData = viewer->getSceneObj()->getSceneData<EventData>();

   mEventData->mObjectMovedSignal.connect(boost::bind(&GrabPlugin::defaultObjectMovedSlot, this, _1, _2));

   InterfaceTrader& if_trader = viewer->getUser()->getInterfaceTrader();
   mWandInterface = if_trader.getWandInterface();

   mIntersectSound.init("intersect");
   mGrabSound.init("grab");

   std::string iov_base_dir;
   vpr::System::getenv("IOV_BASE_DIR", iov_base_dir);
   if ( ! iov_base_dir.empty() )
   {
      fs::path iov_base_dir_path(iov_base_dir, fs::native);
      fs::path shader_subdir_path("share/IOV/data/shaders");
      mShaderSearchPath.push_back(iov_base_dir_path / shader_subdir_path);
   }

   // Configure
   std::string elt_type_name = getElementType();
   jccl::ConfigElementPtr cfg_elt =
      viewer->getConfiguration().getConfigElement(elt_type_name);

   if ( cfg_elt )
   {
      // Configure it
      config(cfg_elt);
   }

   // Initialize the plug-in factories for our strategy plug-in types.
   mPluginFactory = viewer->getPluginFactory();
   mPluginFactory->addScanPath(mStrategyPluginPath);

   // Build the IntersectionStrategy instance.
   try
   {
      IOV_STATUS << "   Loading intersection strategy plug-in '"
                 << mIsectStrategyName << "' ..." << std::flush;
      inf::PluginCreator<inf::IntersectionStrategy>* creator =
         mPluginFactory->getPluginCreator<inf::IntersectionStrategy>(mIsectStrategyName);

      if ( NULL != creator )
      {
         mIsectStrategy = creator->createPlugin();
         mIsectStrategy->init(viewer);
         IOV_STATUS << "[OK]" << std::endl;
      }
      else
      {
         IOV_STATUS << "[ERROR]\nWARNING: No creator for strategy plug-in "
                    << mIsectStrategyName << std::endl;
      }
   }
   catch (std::runtime_error& ex)
   {
      IOV_STATUS << "[ERROR]\nWARNING: Failed to load strategy plug-in "
                 << mIsectStrategyName << ":\n" << ex.what() << std::endl;
   }

   // Build the MoveStrategies
   for (std::vector<std::string>::iterator itr = mMoveStrategyNames.begin();
        itr != mMoveStrategyNames.end(); ++itr)
   {
      try
      {
         IOV_STATUS << "   Loading move strategy plug-in '"
                    << *itr << "' ..." << std::flush;
         inf::PluginCreator<inf::MoveStrategy>* creator =
            mPluginFactory->getPluginCreator<inf::MoveStrategy>(*itr);

         if ( NULL != creator )
         {
            MoveStrategyPtr move_strategy = creator->createPlugin();
            move_strategy->init(viewer);
            mMoveStrategies.push_back(move_strategy);
            IOV_STATUS << "[OK]" << std::endl;
         }
         else
         {
            IOV_STATUS << "[ERROR]\nWARNING: No creator for strategy plug-in "
                       << *itr << std::endl;
         }
      }
      catch (std::runtime_error& ex)
      {
         IOV_STATUS << "[ERROR]\nWARNING: Failed to load strategy plug-in "
                    << *itr << ":\n" << ex.what() << std::endl;
      }
   }

   bool use_scribe(false);

   if ( mEnableShaders )
   {
      try
      {
         mGeomTraverser.extendShaderSearchPath(mShaderSearchPath);

         std::vector<OSG::StateChunkRefPtr> chunks;

         OSG::RefPtr<OSG::BlendChunkPtr> blend_chunk;
         blend_chunk = OSG::BlendChunk::create();
         OSG::beginEditCP(blend_chunk);
            blend_chunk->setSrcFactor(GL_SRC_ALPHA);
            blend_chunk->setDestFactor(GL_ONE);
         OSG::endEditCP(blend_chunk);

         chunks.push_back(OSG::StateChunkRefPtr(blend_chunk.get()));

         OSG::Vec3f isect_color_vec(mIntersectColor[0], mIntersectColor[1],
                                    mIntersectColor[2]);
         inf::GeometryHighlightTraverser::uniform_map_t isect_uniform_params;
         isect_uniform_params["color"]    = isect_color_vec;
         isect_uniform_params["scale"]    = mIsectUniformScale;
         isect_uniform_params["exponent"] = mIsectUniformExponent;
         mIsectHighlightID =
            mGeomTraverser.createSHLMaterial(mIsectVertexShaderFile,
                                             mIsectFragmentShaderFile,
                                             chunks, isect_uniform_params);

         OSG::Vec3f grab_color_vec(mGrabColor[0], mGrabColor[1],
                                   mGrabColor[2]);
         inf::GeometryHighlightTraverser::uniform_map_t grab_uniform_params;
         grab_uniform_params["color"]    = grab_color_vec;
         grab_uniform_params["scale"]    = mGrabUniformScale;
         grab_uniform_params["exponent"] = mGrabUniformExponent;
         mGrabHighlightID =
            mGeomTraverser.createSHLMaterial(mGrabVertexShaderFile,
                                             mGrabFragmentShaderFile,
                                             chunks, grab_uniform_params);
      }
      catch (inf::Exception& ex)
      {
         std::cerr << ex.what() << std::endl;
         std::cout << "Falling back on highlighting through scribing."
                   << std::endl;
         use_scribe = true;
      }
   }
   else
   {
      use_scribe = true;
   }

   if ( use_scribe )
   {
      mIsectHighlightID = mGeomTraverser.createScribeMaterial(false, GL_LINE,
                                                              true, false,
                                                              false, 0.05f,
                                                              1.0f,
                                                              mIntersectColor);
      mGrabHighlightID  = mGeomTraverser.createScribeMaterial(false, GL_LINE,
                                                              true, false,
                                                              false, 0.05f,
                                                              1.0f,
                                                              mGrabColor);
   }
}

void GrabPlugin::updateState(ViewerPtr viewer)
{
   //const ViewPlatform& view_platform = viewer->getUser()->getViewPlatform();

   // Get the wand transformation in virtual platform coordinates.
   const gmtl::Matrix44f vp_M_wand_xform(
      mWandInterface->getWandPos()->getData(viewer->getDrawScaleFactor())
   );

   // Perform intersection tests with all the grabbable objects if and only
   // if we are not already grabbing an object.

   // XXX: Strategy for intersection
   if (NULL != mIsectStrategy.get())
   {
      mIsectStrategy->update(viewer);
   }
   if ( ! mGrabbing )
   {
      // Get the intersected object.
      OSG::TransformNodePtr intersect_obj;
      if (NULL != mIsectStrategy.get())
      {
         mIntersectPoint = gmtl::Point3f();
         intersect_obj = mIsectStrategy->findIntersection(viewer, mIntersectPoint);
      }

      // If the intersected object is different than the one with which the
      // wand intersected during the last frame, we need to make updates to
      // the application and scene state.
      if ( intersect_obj.node() != mIntersectedObj.node() )
      {
         if ( mIntersectedObj.node() != OSG::NullFC )
         {
            // XXX: Is there any cleaner way to do this?
            OSG::NodePtr lit_node = mIntersectedObj.node()->getChild(0);
            mGeomTraverser.removeHighlightMaterial(lit_node,
                                                   mIsectHighlightID);

            // XXX: Send de-select event
         }

         // Change the intersected object to the one we found above.
         mIntersectedObj = intersect_obj;

         // If the new node of mIntersectedObj is non-NULL, then we are
         // intersecting a new object since the last frame.  Set up the
         // highlight node for this new object.
         if ( mIntersectedObj.node() != OSG::NullFC )
         {
            mIntersectSound.trigger();
            mIntersecting = true;

            // XXX: Is there any cleaner way to do this?
            OSG::NodePtr lit_node = mIntersectedObj.node()->getChild(0);

            // Apply mIsectHighlightMaterial.
            mGeomTraverser.addHighlightMaterial(lit_node, mIsectHighlightID);

            // XXX: Send select event.
         }
         // Otherwise, we are intersecting nothing.
         else
         {
            mIntersecting = false;
         }
      }
   }

   // If we are intersecting an object but not grabbing it and the grab
   // button has just been pressed, grab the intersected object.
   if ( mIntersecting && ! mGrabbing &&
        mGrabBtn.test(mWandInterface, gadget::Digital::TOGGLE_ON) )
   {
      mGrabSound.trigger();
      mGrabbing = true;

      // XXX: Is there any cleaner way to do this?
      OSG::NodePtr lit_node = mIntersectedObj.node()->getChild(0);
      mGeomTraverser.swapHighlightMaterial(lit_node, mIsectHighlightID,
                                           mGrabHighlightID);

      gmtl::set(mGrabbed_pobj_M_obj, mIntersectedObj->getMatrix());
   
      if ( !mMoveStrategies.empty() )
      {
         for (std::vector<MoveStrategyPtr>::iterator itr = mMoveStrategies.begin();
              itr != mMoveStrategies.end(); ++itr)
         {
            (*itr)->objectGrabbed(viewer, mIntersectedObj, mIntersectPoint,
                                  vp_M_wand_xform);
         }
      }

      // XXX: Send grab event.
   }
   // If we are grabbing an object and the grab button has just been pressed
   // again, release the grabbed object.
   else if ( mGrabbing &&
             mGrabBtn.test(mWandInterface, gadget::Digital::TOGGLE_ON) )
   {
      mGrabbing = false;

      // We have just released the grabbed object, but we are still
      // intersecting it.  Set the bounding box state back to the
      // intersecting state and clear mIntersectedObj.
      if ( mIntersectedObj.node() != OSG::NullFC )
      {
         // XXX: Is there any cleaner way to do this?
         OSG::NodePtr lit_node = mIntersectedObj.node()->getChild(0);
         mGeomTraverser.swapHighlightMaterial(lit_node, mGrabHighlightID,
                                              mIsectHighlightID);

         gmtl::identity(mGrabbed_pobj_M_obj);

         if ( !mMoveStrategies.empty() )
         {
            for (std::vector<MoveStrategyPtr>::iterator itr = mMoveStrategies.begin();
                 itr != mMoveStrategies.end(); ++itr)
            {
               (*itr)->objectReleased(viewer, mIntersectedObj);
            }
         }

         // XXX: Send release event
      }
   }
}

void GrabPlugin::run(inf::ViewerPtr viewer)
{
   // Move the grabbed object.
   if ( mGrabbing )
   {
      if ( !mMoveStrategies.empty() )
      {
         // Get the wand transformation in virtual world coordinates.
         const gmtl::Matrix44f vp_M_wand_xform(
            mWandInterface->getWandPos()->getData(viewer->getDrawScaleFactor())
         );

         gmtl::Matrix44f new_obj_mat = mGrabbed_pobj_M_obj;

         for (std::vector<MoveStrategyPtr>::iterator itr = mMoveStrategies.begin();
              itr != mMoveStrategies.end(); ++itr)
         {
            // new_obj_mat is gaurenteed to place the object into pobj space
            new_obj_mat = (*itr)->computeMove(viewer, mIntersectedObj, vp_M_wand_xform, new_obj_mat);
         }

         // Send a move event.
         mEventData->mObjectMovedSignal(mIntersectedObj, new_obj_mat);
      }
   }
}

void GrabPlugin::defaultObjectMovedSlot(OSG::TransformNodePtr obj, const gmtl::Matrix44f& newObjMat)
{
   OSG::Matrix obj_mat_osg;
   gmtl::set(obj_mat_osg, newObjMat);
   OSG::beginEditCP(obj, OSG::Transform::MatrixFieldMask);
      obj->setMatrix(obj_mat_osg);
   OSG::endEditCP(obj, OSG::Transform::MatrixFieldMask);
}

bool GrabPlugin::config(jccl::ConfigElementPtr elt)
{
   vprASSERT(elt->getID() == getElementType());

   const unsigned int req_cfg_version(1);

   // Check for correct version of plugin configuration.
   if ( elt->getVersion() < req_cfg_version )
   {
      std::stringstream msg;
      msg << "Configuration of GrabPlugin failed.  Required config "
          << "element version is " << req_cfg_version << ", but element '"
          << elt->getName() << "' is version " << elt->getVersion();
      throw PluginException(msg.str(), IOV_LOCATION);
   }

   const std::string grab_btn_prop("grab_button_nums");
   const std::string isect_prop("intersect_color");
   const std::string grab_prop("grab_color");
   const std::string isect_shader_prop("intersect_shader");
   const std::string isect_scale_prop("intersect_shader_scale");
   const std::string isect_exp_prop("intersect_shader_exponent");
   const std::string grab_shader_prop("grab_shader");
   const std::string grab_scale_prop("grab_shader_scale");
   const std::string grab_exp_prop("grab_shader_exponent");
   const std::string strategy_plugin_path_prop("strategy_plugin_path");
   const std::string isect_strategy_prop("isect_strategy");
   const std::string move_strategy_prop("move_strategy");

   mGrabBtn.configButtons(elt->getProperty<std::string>(grab_btn_prop));

   float isect_color[3];
   float grab_color[3];

   isect_color[0] = elt->getProperty<float>(isect_prop, 0);
   isect_color[1] = elt->getProperty<float>(isect_prop, 1);
   isect_color[2] = elt->getProperty<float>(isect_prop, 2);

   if ( isect_color[0] >= 0.0f && isect_color[0] <= 1.0f &&
        isect_color[1] >= 0.0f && isect_color[1] <= 1.0f &&
        isect_color[2] >= 0.0f && isect_color[2] <= 1.0f )
   {
      mIntersectColor.setValuesRGB(isect_color[0], isect_color[1],
                                   isect_color[2]);
   }
   else
   {
      std::cerr << "WARNING: Ignoring invalid inersection highlight color <"
                << isect_color[0] << "," << isect_color[1] << ","
                << isect_color[2] << ">" << std::endl;
   }

   grab_color[0] = elt->getProperty<float>(grab_prop, 0);
   grab_color[1] = elt->getProperty<float>(grab_prop, 1);
   grab_color[2] = elt->getProperty<float>(grab_prop, 2);

   if ( grab_color[0] >= 0.0f && grab_color[0] <= 1.0f &&
        grab_color[1] >= 0.0f && grab_color[1] <= 1.0f &&
        grab_color[2] >= 0.0f && grab_color[2] <= 1.0f )
   {
      mGrabColor.setValuesRGB(grab_color[0], grab_color[1], grab_color[2]);
   }
   else
   {
      std::cerr << "WARNING: Ignoring invalid grab highlight color <"
                << grab_color[0] << "," << grab_color[1] << ","
                << grab_color[2] << ">" << std::endl;
   }

   const unsigned int num_paths = elt->getNum("shader_search_path");
   for ( unsigned int i = 0; i < num_paths; ++i )
   {
      std::string path =
         vpr::replaceEnvVars(elt->getProperty<std::string>("shader_search_path", i));

      try
      {
         mShaderSearchPath.push_back(fs::path(path, fs::native));
      }
      catch (fs::filesystem_error& ex)
      {
         std::cerr << ex.what() << std::endl;
      }
   }

   mEnableShaders = elt->getProperty<bool>("enable_highlight_shaders");
   mIsectVertexShaderFile =
      vpr::replaceEnvVars(elt->getProperty<std::string>(isect_shader_prop, 0));
   mIsectFragmentShaderFile =
      vpr::replaceEnvVars(elt->getProperty<std::string>(isect_shader_prop, 1));
   mIsectUniformScale    = elt->getProperty<float>(isect_scale_prop, 0);
   mIsectUniformExponent = elt->getProperty<float>(isect_exp_prop, 0);

   mGrabVertexShaderFile =
      vpr::replaceEnvVars(elt->getProperty<std::string>(grab_shader_prop, 0));
   mGrabFragmentShaderFile =
      vpr::replaceEnvVars(elt->getProperty<std::string>(grab_shader_prop, 1));
   mGrabUniformScale    = elt->getProperty<float>(grab_scale_prop, 0);
   mGrabUniformExponent = elt->getProperty<float>(grab_exp_prop, 0);

   // Set up two default search paths:
   //    1. Relative path to './plugins/grab'
   //    2. IOV_BASE_DIR/lib/IOV/plugins/grab
   mStrategyPluginPath.push_back("plugins/grab");

   std::string iov_base_dir;
   if ( vpr::System::getenv("IOV_BASE_DIR", iov_base_dir).success() )
   {
      fs::path iov_base_path(iov_base_dir, fs::native);
      fs::path def_strategy_path = iov_base_path / "lib/IOV/plugins/grab";

      if ( fs::exists(def_strategy_path) )
      {
         std::string def_search_path =
            def_strategy_path.native_directory_string();
         std::cout << "Setting default IOV grab strategy plug-in path: " << def_search_path
                   << std::endl;
         mStrategyPluginPath.push_back(def_search_path);
      }
      else
      {
         std::cerr << "Default IOV grab strategy plug-in path does not exist: "
                   << def_strategy_path.native_directory_string()
                   << std::endl;
      }
   }

   const unsigned int num_plugin_paths(elt->getNum(strategy_plugin_path_prop));
   for ( unsigned int i = 0; i < num_plugin_paths; ++i )
   {
      std::string dir = elt->getProperty<std::string>(strategy_plugin_path_prop, i);
      mStrategyPluginPath.push_back(vpr::replaceEnvVars(dir));
   }

   mIsectStrategyName = elt->getProperty<std::string>(isect_strategy_prop);

   const unsigned int num_move_strategy_names(elt->getNum(move_strategy_prop));
   for ( unsigned int i = 0; i < num_move_strategy_names; ++i)
   {
      std::string move_strategy_name = elt->getProperty<std::string>(move_strategy_prop, i);
      mMoveStrategyNames.push_back(move_strategy_name);
   }


   return true;
}

GrabPlugin::GrabPlugin()
   : mEnableShaders(true)
   , mIsectVertexShaderFile("highlight.vs")
   , mIsectFragmentShaderFile("highlight.fs")
   , mIsectUniformScale(1.0f)
   , mIsectUniformExponent(1.0f)
   , mGrabVertexShaderFile("highlight.vs")
   , mGrabFragmentShaderFile("highlight.fs")
   , mGrabUniformScale(1.0f)
   , mGrabUniformExponent(1.0f)
   , mGrabText("Grab/Release Toggle")
   , mIntersecting(false)
   , mGrabbing(false)
   , mIsectHighlightID(GeometryHighlightTraverser::HIGHLIGHT0)
   , mGrabHighlightID(GeometryHighlightTraverser::HIGHLIGHT1)
   , mIntersectColor(1.0f, 1.0f, 0.0f)
   , mGrabColor(1.0f, 0.0f, 1.0f)
{
   ;
}

struct IncValue
{
   int operator()(int v)
   {
      return v + 1;
   }
};

void GrabPlugin::focusChanged(inf::ViewerPtr viewer)
{
   // If we have focus and our grab/release button is configured, we
   // will update the status panel to include our command.
   if ( isFocused() && mGrabBtn.isConfigured() )
   {
      inf::ScenePtr scene = viewer->getSceneObj();
      StatusPanelPluginDataPtr status_panel_data =
         scene->getSceneData<StatusPanelPluginData>();

      if ( status_panel_data->mStatusPanelPlugin )
      {
         inf::StatusPanel& panel =
            status_panel_data->mStatusPanelPlugin->getPanel();
         if ( ! panel.hasControlText(mGrabBtn.getButtons(), mGrabText) )
         {
            // The button numbers in mGrabBtn are zero-based, but we would like
            // them to be one-based in the status panel display.
            std::vector<int> btns(mGrabBtn.getButtons().size());
            IncValue inc;
            std::transform(mGrabBtn.getButtons().begin(),
                           mGrabBtn.getButtons().end(), btns.begin(), inc);

            panel.addControlText(btns, mGrabText);
         }
      }
   }
   else if ( ! isFocused() )
   {
      inf::ScenePtr scene = viewer->getSceneObj();
      StatusPanelPluginDataPtr status_panel_data =
         scene->getSceneData<StatusPanelPluginData>();

      if ( status_panel_data->mStatusPanelPlugin )
      {
         inf::StatusPanel& panel =
            status_panel_data->mStatusPanelPlugin->getPanel();

         // The button numbers in mGrabBtn are zero-based, but we would like
         // them to be one-based in the status panel display.
         std::vector<int> btns(mGrabBtn.getButtons().size());
         IncValue inc;
         std::transform(mGrabBtn.getButtons().begin(),
                        mGrabBtn.getButtons().end(), btns.begin(), inc);

         panel.removeControlText(btns, mGrabText);
      }
   }
}

}
