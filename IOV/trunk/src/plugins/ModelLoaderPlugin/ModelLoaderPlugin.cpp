#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#endif

#include <gmtl/Matrix.h>
#include <gmtl/Generate.h>
#include <gmtl/External/OpenSGConvert.h>

#include <OpenSG/OSGSwitch.h>
#include <OpenSG/OSGTransform.h>
#include <OpenSG/OSGNode.h>
#include <OpenSG/OSGSceneFileHandler.h>

#include <IOV/Viewer.h>
#include <IOV/WandInterface.h>
#include <IOV/Util/DigitalCommand.h>
#include <IOV/PluginCreator.h>
#include <IOV/InterfaceTrader.h>
#include <IOV/User.h>
#include <IOV/Status.h>
#include <IOV/Util/Exceptions.h>

#include "ModelLoaderPlugin.h"


static inf::PluginCreator<inf::Plugin> sPluginCreator(
   &inf::ModelLoaderPlugin::create, "Model Loader Plug-in"
);

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

IOV_PLUGIN_API(inf::PluginCreatorBase*) getCreator()
{
   return &sPluginCreator;
}
//@}

}


namespace inf
{
   PluginPtr ModelLoaderPlugin::create()
   {
      return PluginPtr(new ModelLoaderPlugin());
   }
   
   std::string ModelLoaderPlugin::getDescription()
   {
      return std::string("Model Loader Plugin");
   }
   
   PluginPtr ModelLoaderPlugin::init(inf::ViewerPtr viewer)
   {      
      const std::string plugin_tkn("model_loader_plugin");
      const std::string units_to_meters_tkn("units_to_meters");
      const std::string position_tkn("position");
      const std::string rotation_tkn("rotation");
      const std::string models_tkn("models");
      const std::string path_tkn("path");
      
      const unsigned int req_cfg_version(1);
      
      jccl::ConfigElementPtr elt = viewer->getConfiguration().getConfigElement(plugin_tkn);

      if(!elt)
      {
         std::stringstream ex_msg;
         ex_msg << "Model loader plugin could not find its configuration.  "
                << "Looking for type: " << plugin_tkn;
         throw PluginException(ex_msg.str(), IOV_LOCATION);
      }
   
      // -- Read configuration -- //
      vprASSERT(elt->getID() == plugin_tkn);
   
      // Check for correct version of plugin configuration
      if(elt->getVersion() < req_cfg_version)
      {
         std::stringstream msg;
         msg << "ModelLoaderPlugin: Configuration failed. Required cfg version: " << req_cfg_version
             << " found:" << elt->getVersion();
         throw PluginException(msg.str(), IOV_LOCATION);
      }
      
      // Get the scaling factor
      float to_meters_scalar = elt->getProperty<float>(units_to_meters_tkn);
      
      // Get the paths to all the models, load them, and add them to the scene
      inf::ScenePtr scene = viewer->getSceneObj();
      OSG::TransformNodePtr scene_xform_root = scene->getTransformRoot();
      
      OSG::beginEditCP(scene_xform_root);
         const unsigned int num_models(elt->getNum(models_tkn));
         for( unsigned int i = 0; i < num_models; ++i )
         {
            jccl::ConfigElementPtr model_elt =
               elt->getProperty<jccl::ConfigElementPtr>(models_tkn, i);
            vprASSERT(model_elt.get() != NULL);
            std::string model_path = model_elt->getProperty<std::string>(path_tkn);
            std::cout << model_path << "." << std::endl;
            OSG::NodePtr model_node = OSG::SceneFileHandler::the().read(model_path.c_str());
            if (model_node != OSG::NullFC)
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
               
               OSG::NodePtr xform_node = OSG::Node::create();
               OSG::TransformPtr xform_core = OSG::Transform::create();
               
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
   
   void ModelLoaderPlugin::updateState(inf::ViewerPtr viewer)
   {
      /* Do nothing. */ ;
   }
   
   void ModelLoaderPlugin::run(inf::ViewerPtr viewer)
   {
      /* Do nothing. */ ;
   }
   
   
}  // namespace inf
