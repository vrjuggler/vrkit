// Copyright (C) Infiscape Corporation 2005-2006

#include <boost/weak_ptr.hpp>

#include <gmtl/gmtl.h>
#include <gmtl/External/OpenSGConvert.h>

#include <IOV/Status.h>
#include <IOV/Scene.h>
#include <IOV/Viewer.h>
#include <IOV/StatusPanel.h>
#include <IOV/StatusPanelPlugin.h>
#include <IOV/StatusPanelData.h>


namespace
{

   const std::string status_panel_elt_tkn("status_panel_plugin");
   const std::string initial_size_prop("initial_size");
   const std::string initial_pos_prop("initial_pos");
   const std::string initial_rot_prop("initial_rot");


class StatusOutputter
{
public:
   StatusOutputter(inf::StatusPanelPluginPtr p)
      : mStatusPlugin(p)
   {;}

   void operator() (const std::string& msg)
   {
      inf::StatusPanelPluginPtr status_panel_plugin(mStatusPlugin);
      status_panel_plugin->getPanel().addStatusMessage(msg);
   }

   inf::StatusPanelPluginWeakPtr mStatusPlugin;
};

}

namespace inf
{

StatusPanelPlugin::StatusPanelPlugin()
   : mStatusPanel(NULL)
{
}

StatusPanelPlugin::~StatusPanelPlugin()
{
   if ( NULL != mStatusPanel )
   {
      delete mStatusPanel;
      mStatusPanel = NULL;
   }
}

inf::PluginPtr StatusPanelPlugin::create()
{
   return inf::PluginPtr(new StatusPanelPlugin);
}

std::string StatusPanelPlugin::getDescription()
{
   return std::string("Status Panel Plugin");
}

StatusPanel& StatusPanelPlugin::getPanel()
{
   return *mStatusPanel;
}

inf::PluginPtr StatusPanelPlugin::init(inf::ViewerPtr viewer)
{
   IOV_STATUS << "StatusPanelPlugin::init: Initializing plugin." << std::endl;

   mStatusPanel = new StatusPanel(viewer->getDrawScaleFactor());

   // Initialize panel
   mStatusPanel->initialize();

   mPanelXformNode = OSG::TransformNodePtr::create();

   // XXX: Read the configuration
   jccl::ConfigElementPtr elt = viewer->getConfiguration().getConfigElement(status_panel_elt_tkn);

   if(elt)
   {
      vprASSERT(elt->getID() == status_panel_elt_tkn);

      const float feet_to_app_units(0.3048f * viewer->getDrawScaleFactor());

      float w,h;
      w = elt->getProperty<float>(initial_size_prop,0);
      h = elt->getProperty<float>(initial_size_prop,1);
      mStatusPanel->setWidthHeight(w * feet_to_app_units,
                                   h * feet_to_app_units);

      float xt = elt->getProperty<float>(initial_pos_prop, 0);
      float yt = elt->getProperty<float>(initial_pos_prop, 1);
      float zt = elt->getProperty<float>(initial_pos_prop, 2);

      float xr = elt->getProperty<float>(initial_rot_prop, 0);
      float yr = elt->getProperty<float>(initial_rot_prop, 1);
      float zr = elt->getProperty<float>(initial_rot_prop, 2);
      gmtl::Coord3fXYZ vp_coord;
      vp_coord.pos().set(xt * feet_to_app_units,
                         yt * feet_to_app_units,
                         zt * feet_to_app_units);
      vp_coord.rot().set(gmtl::Math::deg2Rad(xr),
                         gmtl::Math::deg2Rad(yr),
                         gmtl::Math::deg2Rad(zr));

      gmtl::Matrix44f xform_mat = gmtl::make<gmtl::Matrix44f>(vp_coord); // Set at T*R
      OSG::Matrix xform_osg;
      gmtl::set(xform_osg, xform_mat);

      OSG::beginEditCP(mPanelXformNode);
         mPanelXformNode->setMatrix(xform_osg);
      OSG::endEditCP(mPanelXformNode);
   }

   // Add everything to the tree
   ScenePtr scene_obj = viewer->getSceneObj();
   OSG::GroupNodePtr dec_root = scene_obj->getDecoratorRoot();

   OSG::beginEditCP(mPanelXformNode);
      mPanelXformNode.node()->addChild(mStatusPanel->getPanelRoot());
   OSG::endEditCP(mPanelXformNode);

   OSG::beginEditCP(dec_root);
      dec_root.node()->addChild(mPanelXformNode.node());
   OSG::endEditCP(dec_root);

   // Register with status
   StatusOutputter status_outputter(shared_from_this());
   inf::Status::instance()->addOutputter(status_outputter);

   // Register with scene data
   StatusPanelDataPtr status_panel_data =
      scene_obj->getSceneData<StatusPanelData>();
   status_panel_data->mStatusPanelPlugin = shared_from_this();

   return shared_from_this();
}

void StatusPanelPlugin::update(inf::ViewerPtr)
{
   mStatusPanel->update();        // Do any updates that we need from this frame
}

void StatusPanelPlugin::destroy()
{
   delete this;
}

} // namespace inf
