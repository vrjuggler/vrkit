// vrkit is (C) Copyright 2005-2011
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

#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#endif

#include <sstream>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/assign/list_of.hpp>

#include <OpenSG/OSGMergeGraphOp.h>
#include <OpenSG/OSGTransform.h>
#include <OpenSG/OSGNode.h>
#include <OpenSG/OSGImage.h>
#include <OpenSG/OSGSimpleTexturedMaterial.h>
#include <OpenSG/OSGGeometry.h>
#include <OpenSG/OSGSceneFileHandler.h>
#include <OpenSG/OSGSimpleGeometry.h>

#include <gmtl/gmtl.h>
#include <gmtl/External/OpenSGConvert.h>

#include <vpr/Util/FileUtils.h>
#include <jccl/Config/ConfigElement.h>

#include <vrkit/Scene.h>
#include <vrkit/Viewer.h>
#include <vrkit/Status.h>
#include <vrkit/Version.h>
#include <vrkit/plugin/Creator.h>
#include <vrkit/plugin/Info.h>
#include <vrkit/exceptions/PluginException.h>

#include "LogoPlugin.h"


namespace
{
   const std::string logo_plugin_tkn("logo_plugin");
   const std::string logos_tkn("logos");
   const std::string logo_plugin_logo_tkn("logo_plugin_logo");
   const std::string image_name_tkn("image_name");
   const std::string model_name_tkn("model_name");
   const std::string position_tkn("position");
   const std::string rotation_tkn("rotation");
   const std::string scale_tkn("scale");
}

using namespace boost::assign;

static const vrkit::plugin::Info sInfo(
   "com.infiscape", "LogoPlugin",
   list_of(VRKIT_VERSION_MAJOR)(VRKIT_VERSION_MINOR)(VRKIT_VERSION_PATCH)
);
static vrkit::plugin::Creator<vrkit::viewer::Plugin> sLogoPluginCreator(
   boost::bind(&vrkit::LogoPlugin::create, sInfo)
);

extern "C"
{

/** @name Plug-in Entry points */
//@{
VRKIT_PLUGIN_API(const vrkit::plugin::Info*) getPluginInfo()
{
   return &sInfo;
}

VRKIT_PLUGIN_API(void) getPluginInterfaceVersion(vpr::Uint32& majorVer,
                                                 vpr::Uint32& minorVer)
{
   majorVer = VRKIT_PLUGIN_API_MAJOR;
   minorVer = VRKIT_PLUGIN_API_MINOR;
}

VRKIT_PLUGIN_API(vrkit::plugin::CreatorBase*) getCreator()
{
   return &sLogoPluginCreator;
}
//@}

}

namespace vrkit
{

viewer::PluginPtr LogoPlugin::create(const plugin::Info& info)
{
   return viewer::PluginPtr(new LogoPlugin(info));
}

viewer::PluginPtr LogoPlugin::init(ViewerPtr viewer)
{
   const unsigned int req_cfg_version(1);

   jccl::ConfigElementPtr elt =
      viewer->getConfiguration().getConfigElement(logo_plugin_tkn);

   if ( ! elt )
   {
      std::stringstream ex_msg;
      ex_msg << "Logo plugin could not find its configuration.  "
             << "Looking for type: " << logo_plugin_tkn << "\n"
             << "Aborting plugin configuration.";
      throw PluginException(ex_msg.str(), VRKIT_LOCATION);
   }

   // -- Read configuration -- //
   vprASSERT(elt->getID() == logo_plugin_tkn);

   // Check for correct version of plugin configuration
   if ( elt->getVersion() < req_cfg_version )
   {
      std::stringstream msg;
      msg << "ModeSwitchPlugin: Configuration failed. Required cfg version: "
          << req_cfg_version << " found:" << elt->getVersion();
      throw PluginException(msg.str(), VRKIT_LOCATION);
   }

   VRKIT_STATUS << "Loading LogoPlugin..." << std::endl;

   // Read in all logos
   unsigned int num_logos = elt->getNum(logos_tkn);
   for ( unsigned int i = 0; i < num_logos; ++i )
   {
      jccl::ConfigElementPtr logo_elt =
         elt->getProperty<jccl::ConfigElementPtr>(logos_tkn, i);
      vprASSERT(logo_elt.get() != NULL);
      std::string logo_name = logo_elt->getName();
      VRKIT_STATUS << "   Logo: [" << i << "] " << logo_name << std::endl;

      float xt = logo_elt->getProperty<float>(position_tkn, 0);
      float yt = logo_elt->getProperty<float>(position_tkn, 1);
      float zt = logo_elt->getProperty<float>(position_tkn, 2);

      float xr = logo_elt->getProperty<float>(rotation_tkn, 0);
      float yr = logo_elt->getProperty<float>(rotation_tkn, 1);
      float zr = logo_elt->getProperty<float>(rotation_tkn, 2);

      float xs = logo_elt->getProperty<float>(scale_tkn, 0);
      float ys = logo_elt->getProperty<float>(scale_tkn, 1);
      float zs = logo_elt->getProperty<float>(scale_tkn, 2);

      gmtl::Coord3fXYZ logo_coord;
      logo_coord.pos().set(xt,yt,zt);
      logo_coord.rot().set(gmtl::Math::deg2Rad(xr), gmtl::Math::deg2Rad(yr),
                           gmtl::Math::deg2Rad(zr));
      gmtl::Matrix44f scale_mat =
         gmtl::makeScale<gmtl::Matrix44f>(gmtl::Vec3f(xs,ys,zs));
      gmtl::Matrix44f xform_mat;
      xform_mat = gmtl::make<gmtl::Matrix44f>(logo_coord) * scale_mat;

      // Load up image properties
      std::string image_name =
         logo_elt->getProperty<std::string>(image_name_tkn);
      std::string model_name =
         logo_elt->getProperty<std::string>(model_name_tkn);
      if ( ! image_name.empty() )
      {
         image_name = vpr::replaceEnvVars(image_name);
      }
      if ( ! model_name.empty() )
      {
         model_name = vpr::replaceEnvVars(model_name);
      }

      OSG::NodeRefPtr model_root;

      // If have texture
      if ( ! image_name.empty() )
      {
         VRKIT_STATUS << "      Loading image: " << image_name << std::endl;
         OSG::ImagePtr image = OSG::Image::create();
         image->read(image_name.c_str());

         OSG::SimpleTexturedMaterialPtr mat =
            OSG::SimpleTexturedMaterial::create();
#if OSG_MAJOR_VERSION < 2
         OSG::CPEditor me(mat);
#endif
         mat->setImage(image);
         mat->setLit(false);
         mat->setMagFilter(GL_LINEAR);
         mat->setMinFilter(GL_LINEAR_MIPMAP_LINEAR);

         OSG::GeometryPtr geom = OSG::makePlaneGeo(1.0, 1.0, 1,1);
#if OSG_MAJOR_VERSION < 2
         OSG::CPEditor ge(geom);
#endif
         geom->setMaterial(mat);

         model_root = OSG::Node::create();
#if OSG_MAJOR_VERSION < 2
         OSG::CPEditor ne(model_root);
#endif
         model_root->setCore(geom);
      }
      // we have model file
      else if ( ! model_name.empty() )
      {
         VRKIT_STATUS << "      Loading model: " << model_name << std::endl;
         model_root =
#if OSG_MAJOR_VERSION < 2
            OSG::SceneFileHandler::the().read(model_name.c_str());
#else
            OSG::SceneFileHandler::the()->read(model_name.c_str());
#endif
      }
      else
      {
         VRKIT_STATUS << "      Missing data file name.  Skipping."
                      << std::endl;
         continue;         // Goto next logo
      }

      // Now scale it and save it off
      OSG::TransformNodePtr xform_node = OSG::TransformNodePtr::create();
      OSG::Matrix xform_mat_osg;
      gmtl::set(xform_mat_osg, xform_mat);

#if OSG_MAJOR_VERSION < 2
      OSG::CPEditor xnce(xform_node.core(), OSG::Transform::MatrixFieldMask);
      OSG::CPEditor xne(xform_node.node(), OSG::Node::ChildrenFieldMask);
#endif
      xform_node->setMatrix(xform_mat_osg);
      xform_node.node()->addChild(model_root);

      // Now flatten it
      OSG::NodePtr xform_node_ptr = xform_node.node();
      OSG::MergeGraphOp merge_op;
      merge_op.traverse(xform_node_ptr);

      // Now store it all
      Logo new_logo;
      new_logo.name = logo_name;
      new_logo.xformNode = xform_node;
      mLogos.push_back(new_logo);
   }
   //vprASSERT(mLogos.size() == num_logos);

   // Add them all to the graph.
   ScenePtr scene = viewer->getSceneObj();

   OSG::GroupNodePtr dec_root = scene->getDecoratorRoot();

#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor dre(dec_root.node(), OSG::Node::ChildrenFieldMask);
#endif
   for ( unsigned int i = 0; i < mLogos.size(); ++i )
   {
      dec_root.node()->addChild(mLogos[i].xformNode);
   }

   return shared_from_this();
}

// Nothing to update or do.
void LogoPlugin::update(ViewerPtr)
{
}

} // namespace vrkit
