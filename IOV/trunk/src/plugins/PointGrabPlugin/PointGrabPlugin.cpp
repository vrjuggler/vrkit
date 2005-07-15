// Copyright (C) Infiscape Corporation 2005

#include <string.h>
#include <sstream>
#include <numeric>
#include <algorithm>
#include <stdexcept>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/exception.hpp>

#include <OpenSG/OSGBlendChunk.h>
#include <OpenSG/OSGSHLChunk.h>
#include <OpenSG/OSGPolygonChunk.h>
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
         mIsectHighlightMaterial = createShader(mIsectVertexShaderFile,
                                                mIsectFragmentShaderFile);
         mGrabHighlightMaterial = createShader(mGrabVertexShaderFile,
                                               mGrabFragmentShaderFile);
      }
      catch (std::exception& ex)
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
      // Set up the highlight materials.
      OSG::SimpleMaterialPtr isect_highlight_mat =
         OSG::SimpleMaterial::create();

      OSG::beginEditCP(isect_highlight_mat);
         isect_highlight_mat->setLit(false);
         isect_highlight_mat->setDiffuse(mIntersectColor);
      OSG::endEditCP(isect_highlight_mat);

      OSG::SimpleMaterialPtr grab_highlight_mat =
         OSG::SimpleMaterial::create();

      OSG::beginEditCP(grab_highlight_mat);
         grab_highlight_mat->setLit(false);
         grab_highlight_mat->setDiffuse(mGrabColor);
      OSG::endEditCP(grab_highlight_mat);

      OSG::PolygonChunkPtr scribe_chunk = OSG::PolygonChunk::create();
      OSG::beginEditCP(scribe_chunk);
         scribe_chunk->setFrontMode(GL_LINE);
         scribe_chunk->setOffsetLine(true);
         scribe_chunk->setOffsetFill(false);
         scribe_chunk->setOffsetPoint(false);
         scribe_chunk->setOffsetFactor(0.05f);
         scribe_chunk->setOffsetBias(1.0f);
      OSG::endEditCP(scribe_chunk);

      OSG::beginEditCP(isect_highlight_mat);
         isect_highlight_mat->addChunk(scribe_chunk);
      OSG::endEditCP(isect_highlight_mat);

      OSG::beginEditCP(grab_highlight_mat);
         grab_highlight_mat->addChunk(scribe_chunk);
      OSG::endEditCP(grab_highlight_mat);

      mIsectHighlightMaterial = isect_highlight_mat;
      mGrabHighlightMaterial  = grab_highlight_mat;
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
      inf::CoredTransformPtr intersect_obj;

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

            // Traverse the sub-tree for lit_node and apply
            // mIsectHighlightMaterial accordingly.
            mGeomTraverser.traverse(lit_node);
            OSG::RefPtr<OSG::MaterialPtr> highlight_mat(
               mIsectHighlightMaterial.get()
            );
            mGeomTraverser.addHighlightMaterial(highlight_mat);
         }
         // Otherwise, we are intersecting nothing.
         else
         {
            mGeomTraverser.removeHighlightMaterial();
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
      mGrabbing   = true;

      changeHighlightMaterial(mGrabHighlightMaterial);

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
         changeHighlightMaterial(mIsectHighlightMaterial);
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

   const unsigned int req_cfg_version(2);

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
      vpr::replaceEnvVars(elt->getProperty<std::string>("intersect_shader", 0));
   mIsectFragmentShaderFile =
      vpr::replaceEnvVars(elt->getProperty<std::string>("intersect_shader", 1));

   mGrabVertexShaderFile =
      vpr::replaceEnvVars(elt->getProperty<std::string>("grab_shader", 0));
   mGrabFragmentShaderFile =
      vpr::replaceEnvVars(elt->getProperty<std::string>("grab_shader", 1));

   return true;
}

PointGrabPlugin::PointGrabPlugin()
   : mEnableShaders(true)
   , mIsectVertexShaderFile("highlight.vs")
   , mIsectFragmentShaderFile("highlight.vs")
   , mGrabVertexShaderFile("highlight.vs")
   , mGrabFragmentShaderFile("highlight.vs")
   , mIntersecting(false)
   , mGrabbing(false)
   , mIntersectColor(1.0f, 1.0f, 0.0f)
   , mGrabColor(1.0f, 0.0f, 1.0f)
{
   /* Do nothing. */ ;
}

void PointGrabPlugin::
changeHighlightMaterial(OSG::RefPtr<OSG::ChunkMaterialPtr> newMat)
{
   OSG::RefPtr<OSG::MaterialPtr> highlight_mat(newMat.get());
   mGeomTraverser.changeHighlightMaterial(highlight_mat);
}

OSG::RefPtr<OSG::ChunkMaterialPtr>
PointGrabPlugin::createShader(const std::string& vertexShader,
                              const std::string& fragmentShader)
   throw(std::exception)
{
   fs::path vs_file_path(getCompleteShaderFile(vertexShader));
   fs::path fs_file_path(getCompleteShaderFile(fragmentShader));

   fs::ifstream vs_file(vs_file_path);
   fs::ifstream fs_file(fs_file_path);

   if ( ! vs_file || ! fs_file )
   {
      if ( ! vs_file )
      {
         std::cerr << "WARNING: Could not open '"
                   << vs_file_path.string() << "'" << std::endl;
      }

      if ( ! fs_file )
      {
         std::cerr << "WARNING: Could not open '"
                   << fs_file_path.string() << "'" << std::endl;
      }

      throw std::runtime_error("Failed to find shader programs");
   }
   else
   {
      OSG::RefPtr<OSG::SHLChunkPtr> shl_chunk;
      shl_chunk = OSG::SHLChunk::create();
      {
         OSG::CPEdit(shl_chunk, OSG::ShaderChunk::VertexProgramFieldMask |
                                OSG::ShaderChunk::FragmentProgramFieldMask);

         if ( ! shl_chunk->readVertexProgram(vs_file) )
         {
            throw std::runtime_error("Failed to read vertex program");
         }

         if ( ! shl_chunk->readFragmentProgram(fs_file) )
         {
            throw std::runtime_error("Failed to read fragment program");
         }
      }

      OSG::RefPtr<OSG::BlendChunkPtr> blend_chunk;
      blend_chunk = OSG::BlendChunk::create();
      OSG::beginEditCP(blend_chunk);
         blend_chunk->setSrcFactor(GL_SRC_ALPHA);
         blend_chunk->setDestFactor(GL_ONE);
      OSG::endEditCP(blend_chunk);

      OSG::RefPtr<OSG::ChunkMaterialPtr> material(OSG::ChunkMaterial::create());
      OSG::beginEditCP(material);
         material->addChunk(shl_chunk);
         material->addChunk(blend_chunk);
      OSG::endEditCP(material);

      return material;
   }
}

fs::path PointGrabPlugin::getCompleteShaderFile(const std::string& filename)
{
   fs::path file_path(filename, fs::native);

   if ( ! file_path.is_complete() )
   {
      for ( std::vector<fs::path>::iterator i = mShaderSearchPath.begin();
            i != mShaderSearchPath.end();
            ++i )
      {
         try
         {
            fs::path cur_path(*i / filename);
            if ( fs::exists(cur_path) )
            {
               file_path = cur_path;
               break;
            }
         }
         catch (fs::filesystem_error& ex)
         {
            std::cerr << ex.what() << std::endl;
         }
      }
   }

   return file_path;
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
