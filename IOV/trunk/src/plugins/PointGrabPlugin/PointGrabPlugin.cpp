// Copyright (C) Infiscape Corporation 2005

#include <string.h>
#include <numeric>
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/exception.hpp>

#include <OpenSG/OSGSimpleMaterial.h>

#include <gmtl/Generate.h>
#include <gmtl/Matrix.h>
#include <gmtl/MatrixOps.h>
#include <gmtl/Vec.h>
#include <gmtl/External/OpenSGConvert.h>

#include <vpr/vpr.h>
#include <vpr/System.h>
#include <vpr/Util/FileUtils.h>

#include <IOV/Viewer.h>
#include <IOV/GrabData.h>
#include <IOV/PluginCreator.h>
#include <IOV/PluginPtr.h>
#include <IOV/User.h>
#include <IOV/InterfaceTrader.h>
#include <IOV/WandInterface.h>
#include <IOV/ViewPlatform.h>
#include <IOV/StatusPanel.h>
#include <IOV/StatusPanelPlugin.h>
#include <IOV/Util/Exceptions.h>

#include "PointGrabPlugin.h"


namespace fs = boost::filesystem;

static inf::PluginCreator sPluginCreator(&inf::PointGrabPlugin::create,
                                         "Point Grab Plug-in");

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

IOV_PLUGIN_API(inf::PluginCreator*) getCreator()
{
   return &sPluginCreator;
}
//@}

}

namespace inf
{

void PointGrabPlugin::init(ViewerPtr viewer)
{
   mGrabData =
      viewer->getSceneObj()->getSceneData<GrabData>(GrabData::type_guid);

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

void PointGrabPlugin::updateState(ViewerPtr viewer)
{
   //const ViewPlatform& view_platform = viewer->getUser()->getViewPlatform();

   // Get the wand transformation in virtual platform coordinates.
   const gmtl::Matrix44f vp_M_wand(
      mWandInterface->getWandPos()->getData(viewer->getDrawScaleFactor())
   );

   // Perform intersection tests with all the grabbable objects if and only
   // if we are not already grabbing an object.
   if ( ! mGrabbing )
   {
      OSG::TransformNodePtr intersect_obj;

      const GrabData::object_list_t& objects = mGrabData->getObjects();

      // Find the first object in objects with which the wand intersects.
      GrabData::object_list_t::const_iterator o;
      for ( o = objects.begin(); o != objects.end(); ++o )
      {
         OSG::Matrix world_xform;
         (*o).node()->getParent()->getToWorld(world_xform);
         gmtl::Matrix44f sg_M_vp;
         gmtl::set(sg_M_vp, world_xform);
         gmtl::invert(sg_M_vp);

         // Get the wand transformation in virtual world coordinates,
         // including any transformations in the scene graph below the
         // transformation root.
         const gmtl::Matrix44f sg_M_wand = sg_M_vp * vp_M_wand;
         const gmtl::Vec3f wand_pos_vw(gmtl::makeTrans<gmtl::Vec3f>(sg_M_wand));
         const OSG::Pnt3f wand_point(wand_pos_vw[0], wand_pos_vw[1],
                                     wand_pos_vw[2]);

         const OSG::DynamicVolume& bbox = (*o).node()->getVolume();

         if ( bbox.intersect(wand_point) )
         {
            intersect_obj = *o;
            break;
         }
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
        mGrabBtn.test(gadget::Digital::TOGGLE_ON) )
   {
      mGrabSound.trigger();
      mGrabbing = true;

      // XXX: Is there any cleaner way to do this?
      OSG::NodePtr lit_node = mIntersectedObj.node()->getChild(0);
      mGeomTraverser.swapHighlightMaterial(lit_node, mIsectHighlightID,
                                           mGrabHighlightID);

      // m_wand_M_obj is the offset between the wand and the grabbed object's
      // center point:
      //
      //    m_wand_M_obj = wand_M_vp * vp_M_vw * vw_M_obj
      gmtl::Matrix44f wand_M_vp;
      gmtl::Matrix44f vw_M_obj;

      OSG::Matrix world_xform;
      mIntersectedObj.node()->getParent()->getToWorld(world_xform);
      gmtl::Matrix44f vp_M_vw;
      gmtl::set(vp_M_vw, world_xform);

      gmtl::invert(wand_M_vp, vp_M_wand);
      gmtl::set(vw_M_obj, mIntersectedObj->getMatrix());

      m_wand_M_obj = wand_M_vp * vp_M_vw * vw_M_obj;
   }
   // If we are grabbing an object and the grab button has just been pressed
   // again, release the grabbed object.
   else if ( mGrabbing && mGrabBtn.test(gadget::Digital::TOGGLE_ON) )
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
         gmtl::identity(m_wand_M_obj);
      }
   }
}

void PointGrabPlugin::run(inf::ViewerPtr viewer)
{
   // Move the grabbed object.
   if ( mGrabbing )
   {
      //const ViewPlatform& view_platform = viewer->getUser()->getViewPlatform();

      // vw_M_vp is the current position of the view platform in the virtual
      // world.
      OSG::Matrix world_xform;
      mIntersectedObj.node()->getParent()->getToWorld(world_xform);
      gmtl::Matrix44f vw_M_vp;
      gmtl::set(vw_M_vp, world_xform);
      gmtl::invert(vw_M_vp);

      // Get the wand transformation in virtual world coordinates.
      const gmtl::Matrix44f vp_M_wand(
         mWandInterface->getWandPos()->getData(viewer->getDrawScaleFactor())
      );
      const gmtl::Matrix44f vw_M_wand = vw_M_vp * vp_M_wand;

      gmtl::Matrix44f new_obj_mat = vw_M_wand * m_wand_M_obj;

      osg::Matrix obj_mat_osg;
      gmtl::set(obj_mat_osg, new_obj_mat);
      OSG::beginEditCP(mIntersectedObj, OSG::Transform::MatrixFieldMask);
         mIntersectedObj->setMatrix(obj_mat_osg);
      OSG::endEditCP(mIntersectedObj, OSG::Transform::MatrixFieldMask);
   }
}

bool PointGrabPlugin::config(jccl::ConfigElementPtr elt)
{
   vprASSERT(elt->getID() == getElementType());

   const unsigned int req_cfg_version(3);

   // Check for correct version of plugin configuration.
   if ( elt->getVersion() < req_cfg_version )
   {
      std::stringstream msg;
      msg << "Configuration of PointGrabPlugin failed.  Required config "
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

   configButtons(elt, grab_btn_prop, mGrabBtn);

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

   return true;
}

PointGrabPlugin::PointGrabPlugin()
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
   /* Do nothing. */ ;
}

void PointGrabPlugin::focusChanged(inf::ViewerPtr viewer)
{
   // If we have focus and our grab/release button is configured, we
   // will update the status panel to include our command.
   if ( isFocused() && mGrabBtn.mButtonVec[0] != -1 )
   {
      inf::ScenePtr scene = viewer->getSceneObj();
      StatusPanelPluginDataPtr status_panel_data =
         scene->getSceneData<StatusPanelPluginData>(StatusPanelPluginData::type_guid);

      if ( status_panel_data->mStatusPanelPlugin )
      {
         inf::StatusPanel& panel =
            status_panel_data->mStatusPanelPlugin->getPanel();
         StatusPanel::ControlTextLine line_num =
            (StatusPanel::ControlTextLine) mGrabBtn.mButtonVec[0];
         if ( ! panel.hasControlText(line_num, mGrabText) )
         {
            panel.addControlText(line_num, mGrabText);
         }
      }
   }
   else if ( ! isFocused() )
   {
      inf::ScenePtr scene = viewer->getSceneObj();
      StatusPanelPluginDataPtr status_panel_data =
         scene->getSceneData<StatusPanelPluginData>(StatusPanelPluginData::type_guid);

      if ( status_panel_data->mStatusPanelPlugin )
      {
         inf::StatusPanel& panel =
            status_panel_data->mStatusPanelPlugin->getPanel();

         // Status panel line numbers are 1-based; buttons are 0-based.
         unsigned int line_num = mGrabBtn.mButtonVec[0] + 1;
         panel.removeControlText((StatusPanel::ControlTextLine) line_num,
                                  mGrabText);
      }
   }
}

struct StringToInt
{
   int operator()(const std::string& input)
   {
      return atoi(input.c_str());
   }
};

void PointGrabPlugin::configButtons(jccl::ConfigElementPtr elt,
                                    const std::string& propName,
                                    PointGrabPlugin::DigitalHolder& holder)
{
   holder.mWandIf = mWandInterface;

   std::string btn_str = elt->getProperty<std::string>(propName);

   std::vector<std::string> btn_strings;
   boost::trim(btn_str);
   boost::split(btn_strings, btn_str, boost::is_any_of(", "));
   holder.mButtonVec.resize(btn_strings.size());
   std::transform(btn_strings.begin(), btn_strings.end(),
                  holder.mButtonVec.begin(), StringToInt());
}

bool PointGrabPlugin::DigitalHolder::test(const gadget::Digital::State testState)
{
   if ( mButtonVec.empty() )
   {
      return false;
   }
   else
   {
      mButtonState = testState;
      return std::accumulate(mButtonVec.begin(), mButtonVec.end(), true,
                             *this);
   }
}

bool PointGrabPlugin::DigitalHolder::operator()(bool state, int btn)
{
   return state && mWandIf->getButton(btn)->getData() == mButtonState;
}

}
