// Copyright (C) Infiscape Corporation 2005-2007

#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#endif

#include <boost/bind.hpp>
#include <boost/assign/list_of.hpp>

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
#include <IOV/Version.h>
#include <IOV/Plugin/Info.h>
#include <IOV/Util/Exceptions.h>

#include "ModelSwapPlugin.h"


using namespace boost::assign;

static const inf::plugin::Info sInfo(
   "com.infiscape", "ModelSwapPlugin",
   list_of(IOV_VERSION_MAJOR)(IOV_VERSION_MINOR)(IOV_VERSION_PATCH)
);
static inf::PluginCreator<inf::Plugin> sPluginCreator(
   boost::bind(&inf::ModelSwapPlugin::create, sInfo)
);

extern "C"
{

/** @name Plug-in Entry Points */
//@{
IOV_PLUGIN_API(const inf::plugin::Info*) getPluginInfo()
{
   return &sInfo;
}

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
   PluginPtr ModelSwapPlugin::create(const inf::plugin::Info& info)
   {
      return PluginPtr(new ModelSwapPlugin(info));
   }
   
   std::string ModelSwapPlugin::getDescription()
   {
      return std::string("Model Swap Plugin");
   }
   
   PluginPtr ModelSwapPlugin::init(inf::ViewerPtr viewer)
   {      
      const std::string plugin_tkn("model_swap_plugin");
      const std::string button_tkn("control_button_num");
      const std::string units_to_meters_tkn("units_to_meters");
      const std::string position_tkn("position");
      const std::string rotation_tkn("rotation");
      const std::string model_tkn("model");
      
      const unsigned int req_cfg_version(1);
      
      // Get the wand interface
      InterfaceTrader& if_trader = viewer->getUser()->getInterfaceTrader();
      mWandInterface = if_trader.getWandInterface();
      
      jccl::ConfigElementPtr elt = viewer->getConfiguration().getConfigElement(plugin_tkn);

      if(!elt)
      {
         std::stringstream ex_msg;
         ex_msg << "Model swap plugin could not find its configuration.  "
                << "Looking for type: " << plugin_tkn;
         throw PluginException(ex_msg.str(), IOV_LOCATION);
      }
   
      // -- Read configuration -- //
      vprASSERT(elt->getID() == plugin_tkn);
   
      // Check for correct version of plugin configuration
      if(elt->getVersion() < req_cfg_version)
      {
         std::stringstream msg;
         msg << "ModelSwapPlugin: Configuration failed. Required cfg version: " << req_cfg_version
             << " found:" << elt->getVersion();
         throw PluginException(msg.str(), IOV_LOCATION);
      }

      // Get the button for swapping
      mSwapButton.configure(elt->getProperty<std::string>(button_tkn),
                            mWandInterface);

      // Get the scaling factor
      float to_meters_scalar = elt->getProperty<float>(units_to_meters_tkn);
      
      // Get the paths to all the models, load them, and add them to the switch
      mSwitchNode = OSG::Node::create();
      mSwitchCore = OSG::Switch::create();
      
      OSG::beginEditCP(mSwitchNode);
         mSwitchNode->setCore(mSwitchCore);
         const unsigned int num_models(elt->getNum(model_tkn));
         for( unsigned int i = 0; i < num_models; ++i )
         {
            std::string model_path = elt->getProperty<std::string>(model_tkn, i);
            OSG::NodeRefPtr model_node(OSG::SceneFileHandler::the().read(model_path.c_str()));
            if (model_node != OSG::NullFC)
            {
               mSwitchNode->addChild(model_node);
            }
         }
      OSG::endEditCP(mSwitchNode);
         
      OSG::beginEditCP(mSwitchCore);
         mSwitchCore->setChoice(0);
      OSG::endEditCP(mSwitchCore);
      
      // Set up the model switch transform
      float xt = elt->getProperty<float>(position_tkn, 0);
      float yt = elt->getProperty<float>(position_tkn, 1);
      float zt = elt->getProperty<float>(position_tkn, 2);
      xt *= to_meters_scalar;
      yt *= to_meters_scalar;
      zt *= to_meters_scalar;

      float xr = elt->getProperty<float>(rotation_tkn, 0);
      float yr = elt->getProperty<float>(rotation_tkn, 1);
      float zr = elt->getProperty<float>(rotation_tkn, 2);

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
         xform_node->addChild(mSwitchNode);
      OSG::endEditCP(xform_node);
      
      // add switchable scene to the scene root
      inf::ScenePtr scene = viewer->getSceneObj();

      OSG::TransformNodePtr scene_xform_root = scene->getTransformRoot();
      
      OSG::beginEditCP(scene_xform_root);
         scene_xform_root.node()->addChild(xform_node);
      OSG::endEditCP(scene_xform_root);
      
      return shared_from_this(); 
   }
   
   void ModelSwapPlugin::update(inf::ViewerPtr)
   {
      if ( isFocused() )
      {
         if ( mSwapButton() )
         {
            unsigned int num_models = mSwitchNode->getNChildren();
            unsigned int cur_model = mSwitchCore->getChoice();
            mSwitchCore->setChoice((cur_model + 1) % num_models);
         }
      }
   }

}  // namespace inf
