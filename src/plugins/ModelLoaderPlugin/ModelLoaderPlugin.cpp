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

#include <vector>
#include <boost/bind.hpp>
#include <boost/assign/list_of.hpp>

#include <gmtl/Matrix.h>
#include <gmtl/Generate.h>
#include <gmtl/External/OpenSGConvert.h>

#include <OpenSG/OSGTransform.h>
#include <OpenSG/OSGNode.h>
#include <OpenSG/OSGFieldContainerFactory.h>
#include <OpenSG/OSGSceneFileHandler.h>

#include <jccl/Config/ConfigElement.h>

#include <vrkit/Scene.h>
#include <vrkit/Viewer.h>
#include <vrkit/DynamicSceneObject.h>
#include <vrkit/DynamicSceneObjectTransform.h>
#include <vrkit/Status.h>
#include <vrkit/Version.h>
#include <vrkit/util/CoreTypePredicate.h>
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
   const std::string grabbing_tkn("enable_grabbing");
   const std::string core_type_tkn("core_type");

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
   const float to_meters = elt->getProperty<float>(units_to_meters_tkn);

   // Get the paths to all the models, load them, and add them to the scene
   ScenePtr scene = viewer->getSceneObj();
   OSG::TransformNodePtr scene_xform_root = scene->getTransformRoot();

#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor sxre(scene_xform_root.node(), OSG::Node::ChildrenFieldMask);
#endif

   const unsigned int num_models(elt->getNum(models_tkn));
   for ( unsigned int i = 0; i < num_models; ++i )
   {
      jccl::ConfigElementPtr model_elt =
         elt->getProperty<jccl::ConfigElementPtr>(models_tkn, i);
      vprASSERT(model_elt.get() != NULL);
      const std::string model_path =
         model_elt->getProperty<std::string>(path_tkn);
      OSG::NodeRefPtr model_node(
#if OSG_MAJOR_VERSION < 2
         OSG::SceneFileHandler::the().read(model_path.c_str())
#else
         OSG::SceneFileHandler::the()->read(model_path.c_str())
#endif
      );

      if ( model_node != OSG::NullFC )
      {
         // Set up the model switch transform
         const float xt =
            model_elt->getProperty<float>(position_tkn, 0) * to_meters;
         const float yt =
            model_elt->getProperty<float>(position_tkn, 1) * to_meters;
         const float zt =
            model_elt->getProperty<float>(position_tkn, 2) * to_meters;

         const float xr = model_elt->getProperty<float>(rotation_tkn, 0);
         const float yr = model_elt->getProperty<float>(rotation_tkn, 1);
         const float zr = model_elt->getProperty<float>(rotation_tkn, 2);

         gmtl::Coord3fXYZ coord;
         coord.pos().set(xt,yt,zt);
         coord.rot().set(gmtl::Math::deg2Rad(xr), gmtl::Math::deg2Rad(yr),
                         gmtl::Math::deg2Rad(zr));

         // Set at T*R
         OSG::Matrix xform_mat_osg;
         gmtl::set(xform_mat_osg, gmtl::make<gmtl::Matrix44f>(coord));

         OSG::NodeRefPtr child_root;

         // If this model is supposed to be grabbable, we create a vrkit
         // dynamic scene object for it. As a result, we can be assured
         // that the root node will have a core of type OSG::Transform.
         if ( model_elt->getProperty<bool>(grabbing_tkn) )
         {
            std::vector<OSG::FieldContainerType*> core_types;
            const unsigned int num_types = model_elt->getNum(core_type_tkn);
            for ( unsigned int t = 0; t < num_types; ++t )
            {
               const std::string type_name = 
                  model_elt->getProperty<std::string>(core_type_tkn, i);
               OSG::FieldContainerType* fct =
                  OSG::FieldContainerFactory::the()->findType(
                     type_name.c_str()
                  );

               if ( NULL != fct )
               {
                  core_types.push_back(fct);
               }
               else
               {
                  VRKIT_STATUS << "Skipping unknown type '" << type_name
                               << "'" << std::endl;
               }
            }

            DynamicSceneObjectPtr scene_obj;

            if ( ! core_types.empty() )
            {
               util::CoreTypePredicate pred(core_types);
               scene_obj = DynamicSceneObject::create()->init(model_node,
                                                              pred, true);
            }
            else
            {
               scene_obj =
                  DynamicSceneObjectTransform::create()->init(model_node);
            }

            // scene_obj's root will be added as a child of
            // scene_xform_root.
            child_root = scene_obj->getRoot();
            scene_obj->moveTo(xform_mat_osg);
            viewer->addObject(scene_obj);
         }
         // If this model is not supposed to be grabbable, we examine the
         // core of the root node.
         else
         {
            OSG::NodeCorePtr root_core = model_node->getCore();
            OSG::TransformPtr xform_core =
#if OSG_MAJOR_VERSION < 2
               OSG::TransformPtr::dcast(root_core);
#else
               OSG::cast_dynamic<OSG::TransformPtr>(root_core);
#endif

            // If model_node's core is of type OSG::Transform, then we set
            // set the matrix on that core to be xform_mat_osg.
            if ( OSG::NullFC != xform_core )
            {
#if OSG_MAJOR_VERSION < 2
               OSG::CPEditor xce(xform_core, OSG::Transform::MatrixFieldMask);
#endif
               xform_core->setMatrix(xform_mat_osg);

               // model_node will be added as a child of scene_xform_root.
               child_root = model_node;
            }
            // If model_node's core is not of type OSG::Transform, then we
            // need to make a new node with an OSG::Transform core and make
            // it the parent of model_node.
            else
            {
               OSG::TransformNodePtr xform_node(OSG::Transform::create());

#if OSG_MAJOR_VERSION < 2
               OSG::CPEditor xnce(xform_node.core(),
                                  OSG::Transform::MatrixFieldMask);
#endif
               xform_node->setMatrix(xform_mat_osg);

#if OSG_MAJOR_VERSION < 2
               OSG::CPEditor xne(xform_node.node(),
                                 OSG::Node::ChildrenFieldMask);
#endif
               xform_node.node()->addChild(model_node);

               // xform_node will be added as a child of scene_xform_root.
               child_root = xform_node.node();
            }
         }

         vprASSERT(child_root != OSG::NullFC);

         // The OSG::{begin,end}EditCP() calls for scene_xform_root wrap
         // the for loop that we are in.
         scene_xform_root.node()->addChild(child_root);
      }
   }

   return shared_from_this();
}

void ModelLoaderPlugin::update(ViewerPtr)
{
   /* Do nothing. */ ;
}

}  // namespace vrkit
