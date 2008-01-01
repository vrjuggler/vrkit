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

#ifndef _VRKIT_MATERIAL_CHOOSER_PLUGIN_H_
#define _VRKIT_MATERIAL_CHOOSER_PLUGIN_H_

#include <vrkit/plugin/Config.h>

#include <string>
#include <boost/enable_shared_from_this.hpp>

#include <jccl/Config/ConfigElementPtr.h>

#include <vrkit/viewer/Plugin.h>
#include <vrkit/widget/MaterialChooserPtr.h>


namespace vrkit
{

/**
 * @since 0.48.2
 */
class MaterialChooserPlugin
   : public viewer::Plugin
   , public boost::enable_shared_from_this<MaterialChooserPlugin>
{
private:
   MaterialChooserPlugin(const plugin::Info& info);

public:
   static viewer::PluginPtr create(const plugin::Info& info);

   virtual ~MaterialChooserPlugin();

   virtual viewer::PluginPtr init(ViewerPtr viewer);

   virtual void update(ViewerPtr viewer);

   virtual std::string getDescription()
   {
      return std::string("Material Chooser Plug-in");
   }

private:
   static std::string getElementType()
   {
      return std::string("vrkit_material_chooser_plugin");
   }

   void configure(jccl::ConfigElementPtr elt, const float scaleFactor);

   widget::MaterialChooserPtr mMaterialChooser;
};

}


#endif /* _VRKIT_MATERIAL_CHOOSER_PLUGIN_H_ */
