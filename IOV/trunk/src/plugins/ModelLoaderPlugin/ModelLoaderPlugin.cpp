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

#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#endif

#include <boost/bind.hpp>
#include <boost/assign/list_of.hpp>

#include <gmtl/Matrix.h>
#include <gmtl/Generate.h>
#include <gmtl/External/OpenSGConvert.h>

#include <OpenSG/OSGTransform.h>
#include <OpenSG/OSGNode.h>
#include <OpenSG/OSGSceneFileHandler.h>

#include <jccl/Config/ConfigElement.h>

#include <vrkit/Viewer.h>
#include <vrkit/Version.h>
#include <vrkit/plugin/Creator.h>
#include <vrkit/plugin/Info.h>
#include <vrkit/exceptions/PluginException.h>

#include "ModelLoaderPlugin.h"


using namespace boost::assign;

static const vrkit::plugin::Info sInfo(
   "com.infiscape", "ModelLoaderPlugin",
   list_of(VRKIT_VERSION_MAJOR)(VRKIT_VERSION_MINOR)(VRKIT_VERSION_PATCH)
);
static vrkit::plugin::Creator<vrkit::viewer::Plugin> sPluginCreator(
   boost::bind(&vrkit::ModelLoaderPlugin::create, sInfo)
);

extern "C"
{

/** @name Plug-in Entry Points */
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
   return &sPluginCreator;
}
//@}

}

namespace vrkit
{

viewer::PluginPtr ModelLoaderPlugin::create(const plugin::Info& info)
{
   return viewer::PluginPtr(new ModelLoaderPlugin(info));
}

std::string ModelLoaderPlugin::getDescription()
{
   return std::string("Model Loader Plug-in");
}

viewer::PluginPtr ModelLoaderPlugin::init(ViewerPtr viewer)
{
   const std::string plugin_tkn("model_loader_plugin");
   const std::string units_to_meters_tkn("units_to_meters");
   const std::string position_tkn("position");
   const std::string rotation_tkn("rotation");
   const std::string models_tkn("models");
   const std::string path_tkn("path");

   const unsigned int req_cfg_version(1);

   jccl::ConfigElementPtr elt =
      viewer->getConfiguration().getConfigElement(plugin_tkn);

   if ( !elt )
   {
      std::stringstream ex_msg;
      ex_msg << "Model loader plug-in could not find its configuration.  "
             << "Looking for type: " << plugin_tkn;
      throw PluginException(ex_msg.str(), VRKIT_LOCATION);
   }

   // -- Read configuration -- //
   vprASSERT(elt->getID() == plugin_tkn);

   // Check for correct version of plugin configuration
   if ( elt->getVersion() < req_cfg_version )
   {
      std::stringstream msg;
      msg << "ModelLoaderPlugin: Configuration failed. Required cfg version: "
          << req_cfg_version << " found:" << elt->getVersion();
      throw PluginException(msg.str(), VRKIT_LOCATION);
   }

   // Get the scaling factor
   float to_meters_scalar = elt->getProperty<float>(units_to_meters_tkn);

   // Get the paths to all the models, load them, and add them to the scene
   ScenePtr scene = viewer->getSceneObj();
   OSG::TransformNodePtr scene_xform_root = scene->getTransformRoot();

   OSG::beginEditCP(scene_xform_root);
      const unsigned int num_models(elt->getNum(models_tkn));
      for ( unsigned int i = 0; i < num_models; ++i )
      {
         jccl::ConfigElementPtr model_elt =
            elt->getProperty<jccl::ConfigElementPtr>(models_tkn, i);
         vprASSERT(model_elt.get() != NULL);
         std::string model_path = model_elt->getProperty<std::string>(path_tkn);
         std::cout << model_path << "." << std::endl;
         OSG::NodeRefPtr model_node(
            OSG::SceneFileHandler::the().read(model_path.c_str())
         );

         if ( model_node != OSG::NullFC )
         {
            // Set up the model switch transform
            float xt = model_elt->getProperty<float>(position_tkn, 0);
            float yt = model_elt->getProperty<float>(position_tkn, 1);
            float zt = model_elt->getProperty<float>(position_tkn, 2);
            xt *= to_meters_scalar;
            yt *= to_meters_scalar;
            zt *= to_meters_scalar;

            float xr = model_elt->getProperty<float>(rotation_tkn, 0);
            float yr = model_elt->getProperty<float>(rotation_tkn, 1);
            float zr = model_elt->getProperty<float>(rotation_tkn, 2);

            gmtl::Coord3fXYZ coord;
            coord.pos().set(xt,yt,zt);
            coord.rot().set(gmtl::Math::deg2Rad(xr),
                               gmtl::Math::deg2Rad(yr),
                               gmtl::Math::deg2Rad(zr));

            gmtl::Matrix44f xform_mat = gmtl::make<gmtl::Matrix44f>(coord); // Set at T*R
            OSG::Matrix xform_mat_osg;
            gmtl::set(xform_mat_osg, xform_mat);

            OSG::NodeRefPtr xform_node(OSG::Node::create());
            OSG::TransformRefPtr xform_core(OSG::Transform::create());

            OSG::beginEditCP(xform_core);
               xform_core->setMatrix(xform_mat_osg);
            OSG::endEditCP(xform_core);

            OSG::beginEditCP(xform_node);
               xform_node->setCore(xform_core);
               xform_node->addChild(model_node);
            OSG::endEditCP(xform_node);

            scene_xform_root.node()->addChild(xform_node);
         }
      }
   OSG::endEditCP(scene_xform_root);

   return shared_from_this();
}

void ModelLoaderPlugin::update(ViewerPtr)
{
   /* Do nothing. */ ;
}

}  // namespace vrkit
