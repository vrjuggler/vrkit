// Copyright (C) Infiscape Corporation 2005-2006

#include <iostream>
#include <boost/bind.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/operations.hpp>

#include <OpenSG/OSGBlendChunk.h>

#include <vpr/vpr.h>
#include <vpr/System.h>
#include <vpr/Util/FileUtils.h>
#include <jccl/Config/ConfigElement.h>

#include <IOV/EventData.h>
#include <IOV/SceneObject.h>
#include <IOV/Viewer.h>
#include <IOV/Util/Exceptions.h>

#include <IOV/Util/BasicHighlighter.h>


namespace fs = boost::filesystem;

namespace inf
{

BasicHighlighter::~BasicHighlighter()
{
   mIsectConnection.disconnect();
   mDeIsectConnection.disconnect();
   mSelectConnection.disconnect();
   mDeselectConnection.disconnect();
}

BasicHighlighterPtr BasicHighlighter::init(inf::ViewerPtr viewer)
{
   std::string iov_base_dir;
   vpr::System::getenv("IOV_BASE_DIR", iov_base_dir);

   if ( ! iov_base_dir.empty() )
   {
      fs::path iov_base_dir_path(iov_base_dir, fs::native);
      fs::path shader_subdir_path("share/IOV/data/shaders");
      mShaderSearchPath.push_back(iov_base_dir_path / shader_subdir_path);
   }

   jccl::ConfigElementPtr cfg_elt =
      viewer->getConfiguration().getConfigElement(getElementType());

   if ( cfg_elt )
   {
      try
      {
         // Configure ourselves.
         configure(cfg_elt);
      }
      catch (inf::Exception& ex)
      {
         mEnableShaders = false;
         std::cerr << "Configuration of BasicHighlighter failed:\n"
                   << ex.what() << std::endl;
      }
   }

   bool use_scribe(false);

   if ( mEnableShaders )
   {
      try
      {
         mGeomTraverser.extendShaderSearchPath(mShaderSearchPath);

         std::vector<OSG::StateChunkRefPtr> chunks;

         OSG::BlendChunkRefPtr blend_chunk;
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

   inf::EventDataPtr event_data =
      viewer->getSceneObj()->getSceneData<inf::EventData>();

   // Connect the intersection signal to our slot.
   mIsectConnection = event_data->mObjectIntersectedSignal.connect(100,
      boost::bind(&BasicHighlighter::objectIntersected, this, _1, _2, _3));

   // Connect the de-intersection signal to our slot.
   mDeIsectConnection = event_data->mObjectDeintersectedSignal.connect(100,
      boost::bind(&BasicHighlighter::objectDeintersected, this, _1));

   // NOTE: The boost::function<T> cast in the following statements is to deal
   //       with a baffling compile error when using GCC 4.1.

   // Connect the selection signal to our slot.
   mSelectConnection =
      event_data->mObjectSelectedSignal.connect(
         100,
         (boost::function<inf::Event::ResultType(inf::SceneObjectPtr)>)
            boost::bind(&BasicHighlighter::objectSelected, this, _1, true)
      );

   // Connect the de-selection signal to our slot.
   mDeselectConnection =
      event_data->mObjectDeselectedSignal.connect(
         100,
         (boost::function<inf::Event::ResultType(inf::SceneObjectPtr)>)
            boost::bind(&BasicHighlighter::objectSelected, this, _1, false)
      );

   // Connect the selection signal to our slot.
   mSelectConnection =
      event_data->mObjectPickedSignal.connect(
         100,
         (boost::function<inf::Event::ResultType(inf::SceneObjectPtr)>)
            boost::bind(&BasicHighlighter::objectPicked, this, _1, true)
      );

   // Connect the de-selection signal to our slot.
   mDeselectConnection =
      event_data->mObjectUnpickedSignal.connect(
         100,
         (boost::function<inf::Event::ResultType(inf::SceneObjectPtr)>)
            boost::bind(&BasicHighlighter::objectPicked, this, _1, false)
      );

   return shared_from_this();
}

inf::Event::ResultType
BasicHighlighter::objectIntersected(inf::SceneObjectPtr obj,
                                    inf::SceneObjectPtr,
                                    gmtl::Point3f)
{
   mGeomTraverser.addHighlightMaterial(obj->getRoot().get(),
                                       mIsectHighlightID);

   return inf::Event::CONTINUE;
}

inf::Event::ResultType
BasicHighlighter::objectDeintersected(inf::SceneObjectPtr obj)
{
   mGeomTraverser.removeHighlightMaterial(obj->getRoot().get(),
                                          mIsectHighlightID);

   return inf::Event::CONTINUE;
}



inf::Event::ResultType
BasicHighlighter::objectSelected(inf::SceneObjectPtr obj, const bool selected)
{
   // Switch from the intersection highlight to the grab highlight.
   if ( selected )
   {
      mGeomTraverser.swapHighlightMaterial(obj->getRoot().get(),
                                           mIsectHighlightID,
                                           mGrabHighlightID);
   }
   // Switch from the grab highlight to the intersection highlight.
   else
   {
      mGeomTraverser.swapHighlightMaterial(obj->getRoot().get(),
                                           mGrabHighlightID,
                                           mIsectHighlightID);
   }

   return inf::Event::CONTINUE;
}

inf::Event::ResultType
BasicHighlighter::objectPicked(inf::SceneObjectPtr obj, const bool picked)
{
   // Switch from the intersection highlight to the grab highlight.
   if ( picked )
   {
      mGeomTraverser.swapHighlightMaterial(obj->getRoot().get(),
                                           mIsectHighlightID,
                                           mGrabHighlightID);
   }
   // Switch from the grab highlight to the intersection highlight.
   else
   {
      mGeomTraverser.removeHighlightMaterial(obj->getRoot().get(),
                                             mGrabHighlightID);
   }

   return inf::Event::CONTINUE;
}

BasicHighlighter::BasicHighlighter()
   : mEnableShaders(false)
   , mIsectVertexShaderFile("highlight.vs")
   , mIsectFragmentShaderFile("highlight.fs")
   , mIntersectColor(1.0f, 1.0f, 0.0f)
   , mIsectUniformScale(1.0f)
   , mIsectUniformExponent(1.0f)
   , mGrabVertexShaderFile("highlight.vs")
   , mGrabFragmentShaderFile("highlight.fs")
   , mGrabColor(1.0f, 0.0f, 1.0f)
   , mGrabUniformScale(1.0f)
   , mGrabUniformExponent(1.0f)
   , mIsectHighlightID(inf::GeometryHighlightTraverser::HIGHLIGHT0)
   , mGrabHighlightID(inf::GeometryHighlightTraverser::HIGHLIGHT1)
{
   /* Do nothing. */ ;
}

void BasicHighlighter::configure(jccl::ConfigElementPtr cfgElt)
{
   vprASSERT(cfgElt->getID() == getElementType());

   const unsigned int req_cfg_version(1);

   // Check for correct version of plugin configuration.
   if ( cfgElt->getVersion() < req_cfg_version )
   {
      std::stringstream msg;
      msg << "Configuration of BasicHighlighter failed.  Required config "
          << "element version is " << req_cfg_version << ", but element '"
          << cfgElt->getName() << "' is version " << cfgElt->getVersion();
      throw Exception(msg.str(), IOV_LOCATION);
   }

   const std::string isect_prop("intersect_color");
   const std::string grab_prop("grab_color");
   const std::string isect_shader_prop("intersect_shader");
   const std::string isect_scale_prop("intersect_shader_scale");
   const std::string isect_exp_prop("intersect_shader_exponent");
   const std::string grab_shader_prop("grab_shader");
   const std::string grab_scale_prop("grab_shader_scale");
   const std::string grab_exp_prop("grab_shader_exponent");

   float isect_color[3];
   float grab_color[3];

   isect_color[0] = cfgElt->getProperty<float>(isect_prop, 0);
   isect_color[1] = cfgElt->getProperty<float>(isect_prop, 1);
   isect_color[2] = cfgElt->getProperty<float>(isect_prop, 2);

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

   grab_color[0] = cfgElt->getProperty<float>(grab_prop, 0);
   grab_color[1] = cfgElt->getProperty<float>(grab_prop, 1);
   grab_color[2] = cfgElt->getProperty<float>(grab_prop, 2);

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

   const unsigned int num_paths = cfgElt->getNum("shader_search_path");
   for ( unsigned int i = 0; i < num_paths; ++i )
   {
      std::string path = vpr::replaceEnvVars(
         cfgElt->getProperty<std::string>("shader_search_path", i)
      );

      try
      {
         mShaderSearchPath.push_back(fs::path(path, fs::native));
      }
      catch (fs::filesystem_error& ex)
      {
         std::cerr << ex.what() << std::endl;
      }
   }

   mEnableShaders = cfgElt->getProperty<bool>("enable_highlight_shaders");
   mIsectVertexShaderFile = vpr::replaceEnvVars(
      cfgElt->getProperty<std::string>(isect_shader_prop, 0)
   );
   mIsectFragmentShaderFile = vpr::replaceEnvVars(
      cfgElt->getProperty<std::string>(isect_shader_prop, 1)
   );
   mIsectUniformScale    = cfgElt->getProperty<float>(isect_scale_prop, 0);
   mIsectUniformExponent = cfgElt->getProperty<float>(isect_exp_prop, 0);

   mGrabVertexShaderFile = vpr::replaceEnvVars(
      cfgElt->getProperty<std::string>(grab_shader_prop, 0)
   );
   mGrabFragmentShaderFile = vpr::replaceEnvVars(
      cfgElt->getProperty<std::string>(grab_shader_prop, 1)
   );
   mGrabUniformScale    = cfgElt->getProperty<float>(grab_scale_prop, 0);
   mGrabUniformExponent = cfgElt->getProperty<float>(grab_exp_prop, 0);
}

}
