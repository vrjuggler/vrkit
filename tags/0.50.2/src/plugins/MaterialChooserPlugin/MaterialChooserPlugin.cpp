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

#include <boost/bind.hpp>
#include <boost/assign/list_of.hpp>

#include <OpenSG/OSGMatrix.h>

#include <gmtl/Coord.h>
#include <gmtl/Matrix.h>
#include <gmtl/Math.h>
#include <gmtl/Generate.h>
#include <gmtl/External/OpenSGConvert.h>

#include <jccl/Config/ConfigElement.h>

#include <vrkit/Viewer.h>
#include <vrkit/Scene.h>
#include <vrkit/Status.h>
#include <vrkit/Version.h>
#include <vrkit/scenedata/WidgetData.h>
#include <vrkit/scenedata/MaterialPoolData.h>
#include <vrkit/widget/MaterialChooser.h>
#include <vrkit/plugin/Creator.h>
#include <vrkit/plugin/Info.h>

#include "MaterialChooserPlugin.h"


using namespace boost::assign;

// XXX: Should com.infiscape.WidgetPlugin be identified as a dependency? This
// plug-in is useless without it.
static const vrkit::plugin::Info sInfo(
   "com.infiscape", "MaterialChooserPlugin",
   list_of(VRKIT_VERSION_MAJOR)(VRKIT_VERSION_MINOR)(VRKIT_VERSION_PATCH)
);
static vrkit::plugin::Creator<vrkit::viewer::Plugin> sPluginCreator(
   boost::bind(&vrkit::MaterialChooserPlugin::create, sInfo)
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

MaterialChooserPlugin::MaterialChooserPlugin(const plugin::Info& info)
   : viewer::Plugin(info)
{
   /* Do nothing. */ ;
}

viewer::PluginPtr MaterialChooserPlugin::create(const plugin::Info& info)
{
   return viewer::PluginPtr(new MaterialChooserPlugin(info));
}

MaterialChooserPlugin::~MaterialChooserPlugin()
{
   /* Do nothing. */ ;
}

viewer::PluginPtr MaterialChooserPlugin::init(ViewerPtr viewer)
{
   const float scale_factor(viewer->getDrawScaleFactor());
   mMaterialChooser = widget::MaterialChooser::create();
   mMaterialChooser->init(scale_factor);

   jccl::ConfigElementPtr elt =
      viewer->getConfiguration().getConfigElement(getElementType());

   if ( elt )
   {
      configure(elt, scale_factor);
   }
   else
   {
      const float feet_to_app_units(0.3048f * scale_factor);
      mMaterialChooser->setWidthHeight(feet_to_app_units,
                                       1.5f * feet_to_app_units);
   }

   // Add widget to scene.
   ScenePtr scene = viewer->getSceneObj();
   WidgetDataPtr widget_data = scene->getSceneData<WidgetData>();
   widget_data->addWidget(mMaterialChooser);
   viewer->addObject(mMaterialChooser);

   MaterialPoolDataPtr pool_data = scene->getSceneData<MaterialPoolData>();
   mMaterialChooser->setMaterialPool(pool_data->getMaterialPool());

   return shared_from_this();
}

void MaterialChooserPlugin::update(ViewerPtr)
{
   mMaterialChooser->update();
}

void MaterialChooserPlugin::configure(jccl::ConfigElementPtr elt,
                                      const float scaleFactor)
{
   const std::string size_prop("size");
   const std::string pos_prop("position");
   const std::string rot_prop("rotation");

   // Initialize panel.
   const float feet_to_app_units(0.3048f * scaleFactor);

   float width  = elt->getProperty<float>(size_prop, 0);
   float height = elt->getProperty<float>(size_prop, 1);

   if ( width <= 0.0f )
   {
      VRKIT_STATUS << "ERROR: Invalid width for material chooser " << width
                   << "; using 1.0." << std::endl;
      width = 1.0f;
   }

   if ( height <= 0.0f )
   {
      VRKIT_STATUS << "ERROR: Invalid height for material chooser " << height
                   << "; using 1.5." << std::endl;
      height = 1.5f;
   }

   mMaterialChooser->setWidthHeight(width * feet_to_app_units,
                                    height * feet_to_app_units);

   const float xt = elt->getProperty<float>(pos_prop, 0);
   const float yt = elt->getProperty<float>(pos_prop, 1);
   const float zt = elt->getProperty<float>(pos_prop, 2);

   const float xr = elt->getProperty<float>(rot_prop, 0);
   const float yr = elt->getProperty<float>(rot_prop, 1);
   const float zr = elt->getProperty<float>(rot_prop, 2);

   gmtl::Coord3fXYZ chooser_coord;
   chooser_coord.pos().set(xt * feet_to_app_units, yt * feet_to_app_units,
                           zt * feet_to_app_units);
   chooser_coord.rot().set(gmtl::Math::deg2Rad(xr), gmtl::Math::deg2Rad(yr),
                           gmtl::Math::deg2Rad(zr));

   // Set at T*R
   OSG::Matrix xform_osg;
   gmtl::set(xform_osg, gmtl::make<gmtl::Matrix44f>(chooser_coord));
   mMaterialChooser->moveTo(xform_osg);
}

}
