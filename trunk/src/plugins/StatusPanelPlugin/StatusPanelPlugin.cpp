// vrkit is (C) Copyright 2005-2008
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

#include <boost/weak_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/assign/list_of.hpp>

#include <gmtl/gmtl.h>
#include <gmtl/External/OpenSGConvert.h>

#include <jccl/Config/ConfigElement.h>

#include <vrkit/Status.h>
#include <vrkit/Scene.h>
#include <vrkit/Viewer.h>
#include <vrkit/Version.h>
#include <vrkit/scenedata/StatusPanelData.h>
#include <vrkit/signal/Repository.h>
#include <vrkit/plugin/Creator.h>
#include <vrkit/plugin/Info.h>

#include "StatusPanelPlugin.h"


using namespace boost::assign;

static const vrkit::plugin::Info sInfo(
   "com.infiscape", "StatusPanelPlugin",
   list_of(VRKIT_VERSION_MAJOR)(VRKIT_VERSION_MINOR)(VRKIT_VERSION_PATCH)
);
static vrkit::plugin::Creator<vrkit::viewer::Plugin> sPluginCreator(
   boost::bind(&vrkit::StatusPanelPlugin::create, sInfo)
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

namespace
{

class StatusOutputter
{
public:
   typedef boost::shared_ptr<vrkit::StatusPanelPlugin> plugin_ptr_t;

   StatusOutputter(plugin_ptr_t p)
      : mStatusPlugin(p)
   {
      /* Do nothing. */ ;
   }

   void operator() (const std::string& msg)
   {
      plugin_ptr_t status_panel_plugin(mStatusPlugin);
      status_panel_plugin->getPanel().addStatusMessage(msg);
   }

   boost::weak_ptr<vrkit::StatusPanelPlugin> mStatusPlugin;
};

}

namespace vrkit
{

StatusPanelPlugin::StatusPanelPlugin(const plugin::Info& info)
   : viewer::Plugin(info)
{
   /* Do nothing. */;
}

StatusPanelPlugin::~StatusPanelPlugin()
{
   mVisConn.disconnect();
   mOutputConn.disconnect();
}

viewer::PluginPtr StatusPanelPlugin::create(const plugin::Info& info)
{
   return viewer::PluginPtr(new StatusPanelPlugin(info));
}

std::string StatusPanelPlugin::getDescription()
{
   return std::string("Status Panel Plugin");
}

StatusPanel& StatusPanelPlugin::getPanel()
{
   return mStatusPanel;
}

viewer::PluginPtr StatusPanelPlugin::init(ViewerPtr viewer)
{
   VRKIT_STATUS << "StatusPanelPlugin::init: Initializing plugin."
                << std::endl;

   // Initialize panel
   mStatusPanelView.initialize(viewer->getDrawScaleFactor(), &mStatusPanel);

   mPanelXformNode = OSG::TransformNodePtr::create();

   const std::string status_panel_elt_tkn("status_panel_plugin");
   jccl::ConfigElementPtr elt =
      viewer->getConfiguration().getConfigElement(status_panel_elt_tkn);

   if ( elt )
   {
      vprASSERT(elt->getID() == status_panel_elt_tkn);

      const std::string initial_size_prop("initial_size");
      const std::string initial_pos_prop("initial_pos");
      const std::string initial_rot_prop("initial_rot");

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

#if OSG_MAJOR_VERSION < 2
      OSG::CPEditor pxnce(mPanelXformNode.core(),
                          OSG::Transform::MatrixFieldMask);
#endif
      mPanelXformNode->setMatrix(xform_osg);
   }

   // Add everything to the tree
   ScenePtr scene_obj = viewer->getSceneObj();
   OSG::GroupNodePtr dec_root = scene_obj->getDecoratorRoot();

   mPanelVisSwitchNode = OSG::SwitchNodePtr::create();

#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor pvsne(mPanelVisSwitchNode.node(),
                       OSG::Node::ChildrenFieldMask);
   OSG::CPEditor pvsnce(mPanelVisSwitchNode.core(),
                        OSG::Switch::ChoiceFieldMask);
   OSG::CPEditor pxne(mPanelXformNode.node(), OSG::Node::ChildrenFieldMask);
   OSG::CPEditor dre(dec_root.node(), OSG::Node::ChildrenFieldMask);
#endif

   mPanelVisSwitchNode.node()->addChild(mStatusPanelView.getPanelRoot());
   mPanelVisSwitchNode->setChoice(OSG::Switch::ALL);
   mPanelXformNode.node()->addChild(mPanelVisSwitchNode.node());
   dec_root.node()->addChild(mPanelXformNode.node());

   // Register visibility signal with vrkit::signal::Repository.
   signal::RepositoryPtr sig_repos =
      viewer->getSceneObj()->getSceneData<signal::Repository>();

   typedef boost::signal<void (bool)> sig_type;
   const std::string sig_name("Toggle Status Panel Visibility");

   if ( ! sig_repos->hasSignal(sig_name) )
   {
      sig_repos->addSignal(sig_name, signal::Container<sig_type>::create());
   }

   // Connect new signal to slot after SwitchNode creation
   mVisConn =
      sig_repos->getSignal<sig_type>(sig_name)->connect(
         boost::bind(&StatusPanelPlugin::setVisibility, this, _1)
      );

   // Register with status
   StatusOutputter status_outputter(shared_from_this());
   mOutputConn = Status::instance()->addOutputter(status_outputter);

   // Connect StatusPanel methods to StatusPanelData signals
   StatusPanelDataPtr status_panel_data =
      scene_obj->getSceneData<StatusPanelData>();

   status_panel_data->setHeaderTitle.connect(
      boost::bind(&StatusPanel::setHeaderTitle, &mStatusPanel, _1)
   );

   status_panel_data->setCenterTitle.connect(
      boost::bind(&StatusPanel::setCenterTitle, &mStatusPanel, _1)
   );

   status_panel_data->setBottomTitle.connect(
      boost::bind(&StatusPanel::setBottomTitle, &mStatusPanel, _1)
   );

   status_panel_data->setHeaderText.connect(
      boost::bind(&StatusPanel::setHeaderText, &mStatusPanel, _1)
   );

   status_panel_data->setControlText.connect(
      boost::bind(&StatusPanel::setControlText, &mStatusPanel, _1, _2)
   );

   status_panel_data->addControlText.connect(
      boost::bind(&StatusPanel::addControlText, &mStatusPanel, _1, _2, _3)
   );

   status_panel_data->removeControlText.connect(
      boost::bind(&StatusPanel::removeControlText, &mStatusPanel, _1, _2)
   );

   status_panel_data->addStatusMessage.connect(
      boost::bind(&StatusPanel::addStatusMessage, &mStatusPanel, _1)
   );

   status_panel_data->setWidthHeight.connect(
      boost::bind(&StatusPanelViewOriginal::setWidthHeight, &mStatusPanelView,
                  _1, _2, _3)
   );

   status_panel_data->setStatusHistorySize.connect(
      boost::bind(&StatusPanel::setStatusHistorySize, &mStatusPanel, _1)
   );

   return shared_from_this();
}

void StatusPanelPlugin::update(ViewerPtr)
{
   mStatusPanelView.update();        // Do any updates that we need from this frame
}

void StatusPanelPlugin::setVisibility(bool visible)
{
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor pvsnce(mPanelVisSwitchNode.core(),
                        OSG::Switch::ChoiceFieldMask);
#endif
   mPanelVisSwitchNode->setChoice(visible ? OSG::Switch::ALL
                                          : OSG::Switch::NONE);
}

} // namespace vrkit
