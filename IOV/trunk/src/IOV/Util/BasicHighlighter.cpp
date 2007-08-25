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

#include <iostream>
#include <algorithm>
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

// For GCC 4.1 and newer, require Boost 1.34 or newer.
#if defined(__GNUC__) && \
    ((__GNUC__ == 4 && __GNUC_MINOR__ >= 1) || __GNUC__ > 4)
#  include <boost/version.hpp>
#  if BOOST_VERSION < 103400
#     error "Boost 1.34 or newer is required when using GCC 4.1 or newer"
#  endif
#endif


namespace fs = boost::filesystem;

namespace inf
{

BasicHighlighter::BasicHighlighter()
   : mEnableShaders(false)
   , mIsectVertexShaderFile("highlight.vs")
   , mIsectFragmentShaderFile("highlight.fs")
   , mIntersectColor(1.0f, 1.0f, 0.0f)
   , mIsectUniformScale(1.0f)
   , mIsectUniformExponent(1.0f)
   , mChooseVertexShaderFile("highlight.vs")
   , mChooseFragmentShaderFile("highlight.fs")
   , mChooseColor(0.0f, 1.0f, 1.0f)
   , mChooseUniformScale(1.0f)
   , mChooseUniformExponent(1.0f)
   , mGrabVertexShaderFile("highlight.vs")
   , mGrabFragmentShaderFile("highlight.fs")
   , mGrabColor(1.0f, 0.0f, 1.0f)
   , mGrabUniformScale(1.0f)
   , mGrabUniformExponent(1.0f)
   , mIsectHighlightID(inf::GeometryHighlightTraverser::HIGHLIGHT0)
   , mChooseHighlightID(inf::GeometryHighlightTraverser::HIGHLIGHT1)
   , mGrabHighlightID(inf::GeometryHighlightTraverser::HIGHLIGHT1)
{
   /* Do nothing. */ ;
}

BasicHighlighter::~BasicHighlighter()
{
   std::for_each(mConnections.begin(), mConnections.end(),
                 boost::bind(&boost::signals::connection::disconnect, _1));
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

         OSG::Vec3f choose_color_vec(mChooseColor[0], mChooseColor[1],
                                   mChooseColor[2]);
         inf::GeometryHighlightTraverser::uniform_map_t choose_uniform_params;
         choose_uniform_params["color"]    = choose_color_vec;
         choose_uniform_params["scale"]    = mChooseUniformScale;
         choose_uniform_params["exponent"] = mChooseUniformExponent;
         mChooseHighlightID =
            mGeomTraverser.createSHLMaterial(mChooseVertexShaderFile,
                                             mChooseFragmentShaderFile,
                                             chunks, choose_uniform_params);

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
   mConnections.push_back(
      event_data->objectIntersected.connect(
         100, boost::bind(&BasicHighlighter::objectIntersected, this, _1, _2)
      )
   );

   // Connect the de-intersection signal to our slot.
   mConnections.push_back(
      event_data->objectDeintersected.connect(
         100, boost::bind(&BasicHighlighter::objectDeintersected, this, _1)
      )
   );

   // Connect the selection list expansion signal to our slot.
   mConnections.push_back(
      event_data->selectionListExpanded.connect(
         100,
         boost::bind(&BasicHighlighter::objectChoiceChanged, this, _1, true)
      )
   );

   // Connect the selection list reduction signal to our slot.
   mConnections.push_back(
      event_data->selectionListReduced.connect(
         100,
         boost::bind(&BasicHighlighter::objectChoiceChanged, this, _1, false)
      )
   );

   // Connect the selection signal to our slot.
   mConnections.push_back(
      event_data->objectsSelected.connect(
         100, boost::bind(&BasicHighlighter::objectsSelected, this, _1, true)
      )
   );

   // Connect the de-selection signal to our slot.
   mConnections.push_back(
      event_data->objectsDeselected.connect(
         100, boost::bind(&BasicHighlighter::objectsSelected, this, _1, false)
      )
   );

   // Connect the pick signal to our slot.
   mConnections.push_back(
      event_data->objectPicked.connect(
         100, boost::bind(&BasicHighlighter::objectPicked, this, _1, true)
      )
   );;

   // Connect the un-pick signal to our slot.
   mConnections.push_back(
      event_data->objectUnpicked.connect(
         100, boost::bind(&BasicHighlighter::objectPicked, this, _1, false)
      )
   );

   return shared_from_this();
}

inf::Event::ResultType
BasicHighlighter::objectIntersected(inf::SceneObjectPtr obj,
                                    gmtl::Point3f)
{
   // Only apply the intersection highlight if obj is not currently
   // intersected or grabbed.
   if ( ! isIntersected(obj) && ! isGrabbed(obj) )
   {
      mGeomTraverser.addHighlightMaterial(obj->getRoot().get(),
                                          mIsectHighlightID);
      mIntersectedObjs.push_back(obj);
   }

   return inf::Event::CONTINUE;
}

inf::Event::ResultType
BasicHighlighter::objectDeintersected(inf::SceneObjectPtr obj)
{
   // If obj is not grabbed but is intersected, then we need to remove the
   // intersection highlight. Otherwise, if obj was deintersected while being
   // grabbed, we do not want to make any highlight changes.
   if ( ! isGrabbed(obj) && isIntersected(obj) )
   {
      mIntersectedObjs.erase(std::remove(mIntersectedObjs.begin(),
                                         mIntersectedObjs.end(), obj),
                             mIntersectedObjs.end());

      mGeomTraverser.removeHighlightMaterial(obj->getRoot().get(),
                                             mIsectHighlightID);
   }

   return inf::Event::CONTINUE;
}

inf::Event::ResultType BasicHighlighter::
objectChoiceChanged(const std::vector<inf::SceneObjectPtr>& objs,
                    const bool added)
{
   // When objects are added (added is true) we remove the intersection
   // highlight highlight and add the choose highlight. When objects are
   // removed (added is false), we change the highlight to what makes sense
   // based on the current intersection state.
   std::vector<inf::SceneObjectPtr>::const_iterator o;
   for ( o = objs.begin(); o != objs.end(); ++o )
   {
      OSG::NodePtr root = (*o)->getRoot().get();

      // The given objects have been added to the object selection list. We
      // just swap the intersection highlight for the choose highlight.
      if ( added )
      {
         mChosenObjs.push_back(*o);

         // The current object is intersected, so we replace the intersection
         // highlight with the choose highlight.
         if ( isIntersected(*o) )
         {
            mGeomTraverser.swapHighlightMaterial(root, mIsectHighlightID,
                                                 mChooseHighlightID);
         }
         // The current object is not intersected, so we just add the choose
         // highlight.
         else
         {
            mGeomTraverser.addHighlightMaterial(root, mChooseHighlightID);
         }
      }
      // The given objects have been removed from the object selection list.
      else
      {
         mChosenObjs.erase(
            std::remove(mChosenObjs.begin(), mChosenObjs.end(), *o),
            mChosenObjs.end()
         );

         // The current object is intersected, so we replace the choose
         // highlight with the intersection highlight.
         if ( isIntersected(*o) )
         {
            mGeomTraverser.swapHighlightMaterial(root, mChooseHighlightID,
                                                 mIsectHighlightID);
         }
         // The current object is not intersected, so we just remove the
         // choose highlight.
         else
         {
            mGeomTraverser.removeHighlightMaterial(root, mChooseHighlightID);
         }
      }
   }

   return inf::Event::CONTINUE;
}

inf::Event::ResultType BasicHighlighter::
objectsSelected(const std::vector<inf::SceneObjectPtr>& objs,
                const bool selected)
{
   // When objects are selected (selected is true) we remove the intersection
   // or choose highlight and add the grab highlight. When objects are released
   // (selected is false), we change the highlight to what makes sense based
   // on the current intersection state.
   std::vector<inf::SceneObjectPtr>::const_iterator o;
   for ( o = objs.begin(); o != objs.end(); ++o )
   {
      OSG::NodePtr root = (*o)->getRoot().get();

      // The given objects have been grabbed.
      if ( selected )
      {
         mGrabbedObjs.push_back(*o);

         // Switch from the choose highlight to the grab highlight.
         if ( isChosen(*o) )
         {
            mGeomTraverser.swapHighlightMaterial(root, mChooseHighlightID,
                                                 mGrabHighlightID);
         }
         // Switch from the intersection highlight to the grab highlight.
         else if ( isIntersected(*o) )
         {
            mGeomTraverser.swapHighlightMaterial(root, mIsectHighlightID,
                                                 mGrabHighlightID);
         }
         // Neither the choose nor the intersection highlights are applied
         // to root, so we just add the grab highlight.
         else
         {
            mGeomTraverser.addHighlightMaterial(root, mGrabHighlightID);
         }
      }
      // The given objects have been released.
      else
      {
         mGrabbedObjs.erase(std::remove(mGrabbedObjs.begin(),
                                        mGrabbedObjs.end(), *o),
                            mGrabbedObjs.end());

         // The current object is intersected, so we replace the grab highlight
         // with the intersection highlight.
         if ( isIntersected(*o) )
         {
            mGeomTraverser.swapHighlightMaterial(root, mGrabHighlightID,
                                                 mIsectHighlightID);
         }
         // The current object is not intersected, so we just remove the grab
         // highlight.
         else
         {
            mGeomTraverser.removeHighlightMaterial(root, mGrabHighlightID);
         }
      }
   }

   if ( selected )
   {
      mChosenObjs.clear();
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

bool BasicHighlighter::isIntersected(inf::SceneObjectPtr obj)
{
   return std::find(mIntersectedObjs.begin(), mIntersectedObjs.end(), obj) != mIntersectedObjs.end();
}

bool BasicHighlighter::isChosen(inf::SceneObjectPtr obj)
{
   return std::find(mChosenObjs.begin(), mChosenObjs.end(), obj) != mChosenObjs.end();
}

bool BasicHighlighter::isGrabbed(inf::SceneObjectPtr obj)
{
   return std::find(mGrabbedObjs.begin(), mGrabbedObjs.end(), obj) != mGrabbedObjs.end();
}

void BasicHighlighter::configure(jccl::ConfigElementPtr cfgElt)
{
   vprASSERT(cfgElt->getID() == getElementType());

   const unsigned int req_cfg_version(2);

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
   const std::string choose_prop("choose_color");
   const std::string grab_prop("grab_color");
   const std::string isect_shader_prop("intersect_shader");
   const std::string isect_scale_prop("intersect_shader_scale");
   const std::string isect_exp_prop("intersect_shader_exponent");
   const std::string choose_shader_prop("choose_shader");
   const std::string choose_scale_prop("choose_shader_scale");
   const std::string choose_exp_prop("choose_shader_exponent");
   const std::string grab_shader_prop("grab_shader");
   const std::string grab_scale_prop("grab_shader_scale");
   const std::string grab_exp_prop("grab_shader_exponent");

   setColor(cfgElt, isect_prop, mIntersectColor);
   setColor(cfgElt, choose_prop, mChooseColor);
   setColor(cfgElt, grab_prop, mGrabColor);

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

   mChooseVertexShaderFile = vpr::replaceEnvVars(
      cfgElt->getProperty<std::string>(choose_shader_prop, 0)
   );
   mChooseFragmentShaderFile = vpr::replaceEnvVars(
      cfgElt->getProperty<std::string>(choose_shader_prop, 1)
   );
   mChooseUniformScale    = cfgElt->getProperty<float>(choose_scale_prop, 0);
   mChooseUniformExponent = cfgElt->getProperty<float>(choose_exp_prop, 0);

   mGrabVertexShaderFile = vpr::replaceEnvVars(
      cfgElt->getProperty<std::string>(grab_shader_prop, 0)
   );
   mGrabFragmentShaderFile = vpr::replaceEnvVars(
      cfgElt->getProperty<std::string>(grab_shader_prop, 1)
   );
   mGrabUniformScale    = cfgElt->getProperty<float>(grab_scale_prop, 0);
   mGrabUniformExponent = cfgElt->getProperty<float>(grab_exp_prop, 0);
}

void BasicHighlighter::setColor(jccl::ConfigElementPtr cfgElt,
                                const std::string& propName,
                                OSG::Color3f& color)
{
   float color_vals[3];

   color_vals[0] = cfgElt->getProperty<float>(propName, 0);
   color_vals[1] = cfgElt->getProperty<float>(propName, 1);
   color_vals[2] = cfgElt->getProperty<float>(propName, 2);

   if ( color_vals[0] >= 0.0f && color_vals[0] <= 1.0f &&
        color_vals[1] >= 0.0f && color_vals[1] <= 1.0f &&
        color_vals[2] >= 0.0f && color_vals[2] <= 1.0f )
   {
      color.setValuesRGB(color_vals[0], color_vals[1], color_vals[2]);
   }
   else
   {
      std::cerr << "WARNING: Ignoring invalid highlight color <"
                << color_vals[0] << "," << color_vals[1] << ","
                << color_vals[2] << "> from property " << propName
                << std::endl;
   }
}

}
