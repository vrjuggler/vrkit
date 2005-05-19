//#include <stdio.h>      /* For EOF */
//#include <sstream>
#include <stdexcept>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

#include <OpenSG/OSGSHLChunk.h>

#include <gmtl/Generate.h>
#include <gmtl/Matrix.h>
#include <gmtl/MatrixOps.h>
#include <gmtl/Vec.h>
#include <gmtl/External/OpenSGConvert.h>

#include <vpr/vpr.h>
#include <vpr/System.h>

#include <IOV/Viewer.h>
#include <IOV/GrabData.h>
#include <IOV/PluginCreator.h>
#include <IOV/PluginPtr.h>
#include <IOV/User.h>
#include <IOV/InterfaceTrader.h>
#include <IOV/WandInterface.h>
#include <IOV/ViewPlatform.h>
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

const inf::CoredTransformPtr PointGrabPlugin::sEmptyCoredXformNode;

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
      mShaderDir = iov_base_dir_path / shader_subdir_path;
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

   std::cout << "Shader directory: " << mShaderDir.string() << std::endl;

   try
   {
      fs::path vs_file_path(mShaderDir / "highlight.vs");
      fs::ifstream vs_file(vs_file_path);
      fs::path fs_file_path(mShaderDir / "highlight.fs");
      fs::ifstream fs_file(fs_file_path);

      if ( ! vs_file || ! fs_file )
      {
         if ( ! vs_file )
         {
            std::cerr << "WARNING: Could not open '" << vs_file_path.string()
                      << "'" << std::endl;
         }

         if ( ! fs_file )
         {
            std::cerr << "WARNING: Could not open '" << fs_file_path.string()
                      << "'" << std::endl;
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

         mIsectHighlightMaterial = OSG::ChunkMaterial::create();
         OSG::beginEditCP(mIsectHighlightMaterial);
            mIsectHighlightMaterial->addChunk(shl_chunk);
            mIsectHighlightMaterial->addChunk(blend_chunk);
         OSG::endEditCP(mIsectHighlightMaterial);

         mGrabHighlightMaterial = OSG::ChunkMaterial::create();
         OSG::beginEditCP(mGrabHighlightMaterial);
            mGrabHighlightMaterial->addChunk(shl_chunk);
            mGrabHighlightMaterial->addChunk(blend_chunk);
         OSG::endEditCP(mGrabHighlightMaterial);

         // Give mCoredHighlightNode a node with no core.  The core will come
         // in later.
         mCoredHighlightNode = inf::CoredGeomPtr(OSG::Node::create());
         mUsingShader = true;
      }
   }
   catch (std::exception& ex)
   {
      std::cerr << ex.what() << std::endl;
      std::cout << "Falling back on bounding box highlighting." << std::endl;

      // Set up the highlight materials.
      OSG::SimpleMaterialPtr isect_highlight_mat =
         OSG::SimpleMaterial::create();

      OSG::beginEditCP(isect_highlight_mat);
         isect_highlight_mat->setLit(false);
         isect_highlight_mat->setDiffuse(mIntersectColor);
      OSG::endEditCP(isect_highlight_mat);

      mIsectHighlightMaterial = isect_highlight_mat;

      OSG::SimpleMaterialPtr grab_highlight_mat =
         OSG::SimpleMaterial::create();

      OSG::beginEditCP(grab_highlight_mat);
         grab_highlight_mat->setLit(false);
         grab_highlight_mat->setDiffuse(mGrabColor);
      OSG::endEditCP(grab_highlight_mat);

      mIsectHighlightMaterial = isect_highlight_mat;
      mGrabHighlightMaterial  = grab_highlight_mat;

      // Set up the highlight bounding box.
      OSG::GeoPTypesPtr type = OSG::GeoPTypesUI8::create();
      OSG::beginEditCP(type);
         type->push_back(GL_LINE_STRIP);
         type->push_back(GL_LINES);
      OSG::endEditCP(type);

      OSG::GeoPLengthsPtr lens = OSG::GeoPLengthsUI32::create();
      OSG::beginEditCP(lens);
         lens->push_back(10);
         lens->push_back(6);
      OSG::endEditCP(lens);

      OSG::GeoIndicesUI32Ptr index = OSG::GeoIndicesUI32::create();
      OSG::beginEditCP(index);
         index->getFieldPtr()->push_back(0);
         index->getFieldPtr()->push_back(1);
         index->getFieldPtr()->push_back(3);
         index->getFieldPtr()->push_back(2);
         index->getFieldPtr()->push_back(0);
         index->getFieldPtr()->push_back(4);
         index->getFieldPtr()->push_back(5);
         index->getFieldPtr()->push_back(7);
         index->getFieldPtr()->push_back(6);
         index->getFieldPtr()->push_back(4);

         index->getFieldPtr()->push_back(1);
         index->getFieldPtr()->push_back(5);
         index->getFieldPtr()->push_back(2);
         index->getFieldPtr()->push_back(6);
         index->getFieldPtr()->push_back(3);
         index->getFieldPtr()->push_back(7);
      OSG::endEditCP(index);

      mHighlightPoints = OSG::GeoPositions3f::create();
      OSG::beginEditCP(mHighlightPoints);
         mHighlightPoints->push_back(OSG::Pnt3f(-1, -1, -1));
         mHighlightPoints->push_back(OSG::Pnt3f( 1, -1, -1));
         mHighlightPoints->push_back(OSG::Pnt3f(-1,  1, -1));
         mHighlightPoints->push_back(OSG::Pnt3f( 1,  1, -1));
         mHighlightPoints->push_back(OSG::Pnt3f(-1, -1,  1));
         mHighlightPoints->push_back(OSG::Pnt3f( 1, -1,  1));
         mHighlightPoints->push_back(OSG::Pnt3f(-1,  1,  1));
         mHighlightPoints->push_back(OSG::Pnt3f( 1,  1,  1));
      OSG::endEditCP(mHighlightPoints);

      OSG::GeometryPtr geo = OSG::Geometry::create();
      OSG::beginEditCP(geo);
         geo->setTypes(type);
         geo->setLengths(lens);
         geo->setIndices(index);
         geo->setPositions(mHighlightPoints);
      OSG::endEditCP(geo);
      OSG::addRefCP(geo);

      mCoredHighlightNode = inf::CoredGeomPtr(geo);
   }
}

void PointGrabPlugin::updateState(ViewerPtr viewer)
{
   const ViewPlatform& view_platform = viewer->getUser()->getViewPlatform();

   // vw_M_vp is the current position of the view platform in the virtual
   // world.
   const gmtl::Matrix44f& vw_M_vp(view_platform.getCurPos());

   // Get the wand transformation in virtual world coordinates.
   const gmtl::Matrix44f vp_M_wand(
      mWandInterface->getWandPos()->getData(viewer->getDrawScaleFactor())
   );
   const gmtl::Matrix44f vw_M_wand = vw_M_vp * vp_M_wand;

   // Perform intersection tests with all the grabbable objects if and only
   // if we are not already grabbing an object.
   if ( ! mGrabbing )
   {
      const gmtl::Vec3f wand_pos_vw = gmtl::makeTrans<gmtl::Vec3f>(vw_M_wand);
      const OSG::Pnt3f wand_point(wand_pos_vw[0], wand_pos_vw[1],
                                  wand_pos_vw[2]);

      inf::CoredTransformPtr intersect_obj;

      const GrabData::object_list_t& objects = mGrabData->getObjects();

      // Find the first object in objects with which the wand intersects.
      GrabData::object_list_t::const_iterator o;
      for ( o = objects.begin(); o != objects.end(); ++o )
      {
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
         // If mIntersectedObj's node is non-NULL, then we need to detach
         // the highlight node from mIntersectedObj.
         if ( mIntersectedObj.node() != OSG::NullFC )
         {
            OSG::beginEditCP(mIntersectedObj, OSG::Node::ChildrenFieldMask);
               mIntersectedObj.node()->subChild(mCoredHighlightNode);
            OSG::endEditCP(mIntersectedObj, OSG::Node::ChildrenFieldMask);
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

            OSG::NodePtr lit_node = mIntersectedObj.node()->getChild(0);

            if ( mUsingShader )
            {
               OSG::NodePtr highlight_parent =
                  mCoredHighlightNode.node()->getParent();

               if ( highlight_parent != mIntersectedObj.node() )
               {
                  // XXX: Is there any cleaner way to do this?
                  mCoredHighlightNode =
                     mIntersectedObj.node()->getChild(0)->clone();
               }
            }

            OSG::beginEditCP(mCoredHighlightNode,
                             OSG::Geometry::MaterialFieldMask);
               mCoredHighlightNode->setMaterial(mIsectHighlightMaterial);
            OSG::endEditCP(mCoredHighlightNode,
                           OSG::Geometry::MaterialFieldMask);

            OSG::beginEditCP(mIntersectedObj, OSG::Node::ChildrenFieldMask);
               mIntersectedObj.node()->addChild(mCoredHighlightNode);
            OSG::endEditCP(mIntersectedObj, OSG::Node::ChildrenFieldMask);

            if ( ! mUsingShader )
            {
               updateHighlight(lit_node);
            }
         }
         // Otherwise, we are intersecting nothing.
         else
         {
            mIntersecting = false;
            mIntersectedObj = sEmptyCoredXformNode;
         }
      }
   }

   gadget::DigitalInterface& grab_button =
      mWandInterface->getButton(GRAB_BUTTON);

   // If we are intersecting an object but not grabbing it and the grab
   // button has just been pressed, grab the intersected object.
   if ( mIntersecting && ! mGrabbing &&
        grab_button->getData() == gadget::Digital::TOGGLE_ON )
   {
      mGrabSound.trigger();
      mGrabbing   = true;
      mGrabbedObj = mIntersectedObj;

      OSG::beginEditCP(mCoredHighlightNode, OSG::Geometry::MaterialFieldMask);
         mCoredHighlightNode->setMaterial(mGrabHighlightMaterial);
      OSG::endEditCP(mCoredHighlightNode, OSG::Geometry::MaterialFieldMask);

      // m_wand_M_obj is the offset between the wand and the grabbed object's
      // center point:
      //
      //    m_wand_M_obj = wand_M_vp * vp_M_vw * vw_M_obj
      gmtl::Matrix44f wand_M_vp;
      gmtl::Matrix44f vp_M_vw;
      gmtl::Matrix44f vw_M_obj;

      gmtl::invert(wand_M_vp, vp_M_wand);
      gmtl::invert(vp_M_vw, vw_M_vp);
      gmtl::set(vw_M_obj, mGrabbedObj->getMatrix());

      m_wand_M_obj = wand_M_vp * vp_M_vw * vw_M_obj;
   }
   // If we are grabbing an object and the grab button has just been pressed
   // again, release the grabbed object.
   else if ( mGrabbing &&
             grab_button->getData() == gadget::Digital::TOGGLE_ON )
   {
      mGrabbing = false;

      // If mGrabbedObj's node is non-NULL, then we have just let go of the
      // object we were grabbing.  Set the bounding box state back to the
      // intersecting state and clear mGrabbedObj.
      if ( mGrabbedObj.node() != OSG::NullFC )
      {
         OSG::beginEditCP(mCoredHighlightNode,
                          OSG::Geometry::MaterialFieldMask);
            mCoredHighlightNode->setMaterial(mIsectHighlightMaterial);
         OSG::endEditCP(mCoredHighlightNode, OSG::Geometry::MaterialFieldMask);

         mGrabbedObj = sEmptyCoredXformNode;
         gmtl::identity(m_wand_M_obj);
      }
   }
}

void PointGrabPlugin::run(inf::ViewerPtr viewer)
{
   // Move the grabbed object.
   if ( mGrabbing )
   {
      const ViewPlatform& view_platform = viewer->getUser()->getViewPlatform();

      // vw_M_vp is the current position of the view platform in the virtual
      // world.
      const gmtl::Matrix44f& vw_M_vp(view_platform.getCurPos());

      // Get the wand transformation in virtual world coordinates.
      const gmtl::Matrix44f vp_M_wand(
         mWandInterface->getWandPos()->getData(viewer->getDrawScaleFactor())
      );
      const gmtl::Matrix44f vw_M_wand = vw_M_vp * vp_M_wand;

      gmtl::Matrix44f cur_obj_mat;
      gmtl::set(cur_obj_mat, mGrabbedObj->getMatrix());
      gmtl::Matrix44f new_obj_mat = vw_M_wand * m_wand_M_obj;

      osg::Matrix obj_mat_osg;
      gmtl::set(obj_mat_osg, new_obj_mat);
      OSG::beginEditCP(mGrabbedObj, OSG::Transform::MatrixFieldMask);
         mGrabbedObj->setMatrix(obj_mat_osg);
      OSG::endEditCP(mGrabbedObj, OSG::Transform::MatrixFieldMask);
   }
}

bool PointGrabPlugin::config(jccl::ConfigElementPtr elt)
{
   vprASSERT(elt->getID() == getElementType());

   const std::string isect_prop("intersect_color");
   const std::string grab_prop("grab_color");

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

   return true;
}

// The implementation of this helper method is adapted from
// OSG::SimpleSceneManager::updateHighlight() in OpenSG 1.4.0.
void PointGrabPlugin::updateHighlight(OSG::NodePtr highlightNode)
{
   if ( highlightNode == OSG::NullFC )
   {
      return;
   }

   const OSG::DynamicVolume& vol = highlightNode->getVolume();

   OSG::Pnt3f min, max;
   vol.getBounds(min, max);

   OSG::beginEditCP(mHighlightPoints);
      mHighlightPoints->setValue(OSG::Pnt3f(min[0], min[1], min[2]), 0);
      mHighlightPoints->setValue(OSG::Pnt3f(max[0], min[1], min[2]), 1);
      mHighlightPoints->setValue(OSG::Pnt3f(min[0], max[1], min[2]), 2);
      mHighlightPoints->setValue(OSG::Pnt3f(max[0], max[1], min[2]), 3);
      mHighlightPoints->setValue(OSG::Pnt3f(min[0], min[1], max[2]), 4);
      mHighlightPoints->setValue(OSG::Pnt3f(max[0], min[1], max[2]), 5);
      mHighlightPoints->setValue(OSG::Pnt3f(min[0], max[1], max[2]), 6);
      mHighlightPoints->setValue(OSG::Pnt3f(max[0], max[1], max[2]), 7);
   OSG::endEditCP(mHighlightPoints);

   OSG::beginEditCP(mCoredHighlightNode, OSG::Geometry::PositionsFieldMask);
   OSG::endEditCP(mCoredHighlightNode, OSG::Geometry::PositionsFieldMask);
}

}
