// Copyright (C) Infiscape Corporation 2005-2007

#include <boost/weak_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/assign/list_of.hpp>

#include <gmtl/gmtl.h>
#include <gmtl/External/OpenSGConvert.h>

#include <jccl/Config/ConfigElement.h>

#include <IOV/SignalRepository.h>
#include <IOV/PluginCreator.h>
#include <IOV/Status.h>
#include <IOV/Scene.h>
#include <IOV/Viewer.h>
#include <IOV/StatusPanelData.h>
#include <IOV/Version.h>
#include <IOV/Plugin/Info.h>

#include "StatusPanelPlugin.h"


using namespace boost::assign;

static const inf::plugin::Info sInfo(
   "com.infiscape", "StatusPanelPlugin",
   list_of(IOV_VERSION_MAJOR)(IOV_VERSION_MINOR)(IOV_VERSION_PATCH)
);
static inf::PluginCreator<inf::Plugin> sPluginCreator(
   boost::bind(&inf::StatusPanelPlugin::create, sInfo)
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
   typedef boost::shared_ptr<StatusPanelPlugin> StatusPanelPluginPtr;
   typedef boost::weak_ptr<StatusPanelPlugin> StatusPanelPluginWeakPtr;
}

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

StatusPanelPlugin::StatusPanelPlugin(const inf::plugin::Info& info)
   : Plugin(info)
{
   /* Do nothing. */;
}

StatusPanelPlugin::~StatusPanelPlugin()
{
   mVisConn.disconnect();
   mOutputConn.disconnect();
}

inf::PluginPtr StatusPanelPlugin::create(const inf::plugin::Info& info)
{
   return inf::PluginPtr(new StatusPanelPlugin(info));
}

std::string StatusPanelPlugin::getDescription()
{
   return std::string("Status Panel Plugin");
}

StatusPanel& StatusPanelPlugin::getPanel()
{
   return mStatusPanel;
}

inf::PluginPtr StatusPanelPlugin::init(inf::ViewerPtr viewer)
{
   IOV_STATUS << "StatusPanelPlugin::init: Initializing plugin." << std::endl;

   // Initialize panel
   mStatusPanelView.initialize(viewer->getDrawScaleFactor(), &mStatusPanel);

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
      mStatusPanelView.setWidthHeight(w * feet_to_app_units,
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

   mPanelVisSwitchNode = OSG::SwitchNodePtr::create();

   OSG::beginEditCP(mPanelVisSwitchNode);
      mPanelVisSwitchNode.node()->addChild(mStatusPanelView.getPanelRoot());
   OSG::endEditCP(mPanelVisSwitchNode);

   OSG::beginEditCP(mPanelVisSwitchNode, OSG::Switch::ChoiceFieldMask);
      mPanelVisSwitchNode->setChoice(OSG::Switch::ALL);
   OSG::endEditCP(mPanelVisSwitchNode, OSG::Switch::ChoiceFieldMask);

   OSG::beginEditCP(mPanelXformNode);
      mPanelXformNode.node()->addChild(mPanelVisSwitchNode.node());
   OSG::endEditCP(mPanelXformNode);

   OSG::beginEditCP(dec_root);
      dec_root.node()->addChild(mPanelXformNode.node());
   OSG::endEditCP(dec_root);

   // Register visibility signal with SignalRepository
   inf::SignalRepositoryPtr sig_repos =
      viewer->getSceneObj()->getSceneData<SignalRepository>();

   typedef boost::signal<void (bool)> sig_type;
   std::string sig_name("Toggle Status Panel Visibility");

   if( ! sig_repos->hasSignal(sig_name) )
   {
      sig_repos->addSignal(sig_name,
			   SignalContainer<sig_type>::create());
   }

   // Connect new signal to slot after SwitchNode creation
   mVisConn = sig_repos->getSignal<sig_type>(sig_name)->connect(
      boost::bind(&inf::StatusPanelPlugin::setVisibility, this, _1)
      );

   // Register with status
   StatusOutputter status_outputter(shared_from_this());
   mOutputConn = inf::Status::instance()->addOutputter(status_outputter);

   // Connect StatusPanel methods to StatusPanelData signals
   StatusPanelDataPtr status_panel_data =
      scene_obj->getSceneData<StatusPanelData>();

   status_panel_data->mSetHeaderTitle.connect(
      boost::bind(&StatusPanel::setHeaderTitle, &mStatusPanel, _1));

   status_panel_data->mSetCenterTitle.connect(
      boost::bind(&StatusPanel::setCenterTitle, &mStatusPanel, _1));

   status_panel_data->mSetBottomTitle.connect(
      boost::bind(&StatusPanel::setBottomTitle, &mStatusPanel, _1));

   status_panel_data->mSetHeaderText.connect(
      boost::bind(&StatusPanel::setHeaderText, &mStatusPanel, _1));

   status_panel_data->mSetControlText.connect(
      boost::bind(
         &StatusPanel::setControlText, &mStatusPanel, _1, _2)
      );

   status_panel_data->mAddControlText.connect(
      boost::bind(
      &StatusPanel::addControlText, &mStatusPanel, _1, _2, _3)
      );

   status_panel_data->mRemoveControlText.connect(
      boost::bind(
         &StatusPanel::removeControlText, &mStatusPanel, _1, _2)
      );

   status_panel_data->mHasControlText.connect(
      boost::bind(
         &StatusPanel::hasControlText, &mStatusPanel, _1, _2, _3)
      );

   status_panel_data->mAddStatusMessage.connect(
      boost::bind(&StatusPanel::addStatusMessage, &mStatusPanel, _1));

   status_panel_data->mSetWidthHeight.connect(
      boost::bind(&StatusPanelViewOriginal::setWidthHeight, &mStatusPanelView, _1, _2, _3));

   status_panel_data->mSetStatusHistorySize.connect(
      boost::bind(&StatusPanel::setStatusHistorySize, &mStatusPanel, _1));

   return shared_from_this();
}

void StatusPanelPlugin::update(inf::ViewerPtr)
{
   mStatusPanelView.update();        // Do any updates that we need from this frame
}

void StatusPanelPlugin::setVisibility(bool visible)
{
   OSG::beginEditCP(mPanelVisSwitchNode, OSG::Switch::ChoiceFieldMask);
      if( visible )
      {
	 mPanelVisSwitchNode->setChoice(OSG::Switch::ALL);
      }
      else
      {
	 mPanelVisSwitchNode->setChoice(OSG::Switch::NONE);
      }
   OSG::endEditCP(mPanelVisSwitchNode, OSG::Switch::ChoiceFieldMask);
}


void StatusPanelPlugin::destroy()
{
   delete this;
}

} // namespace inf
