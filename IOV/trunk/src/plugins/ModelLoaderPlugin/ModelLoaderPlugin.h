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

#ifndef _INF_MODEL_LOADER_PLUGIN_H_
#define _INF_MODEL_LOADER_PLUGIN_H_

#include <string>
#include <boost/enable_shared_from_this.hpp>

#include <IOV/Plugin/PluginConfig.h>
#include <IOV/Plugin.h>

namespace inf
{

class ModelLoaderPlugin
   : public inf::Plugin
   , public boost::enable_shared_from_this<ModelLoaderPlugin>
{
protected:
   ModelLoaderPlugin(const inf::plugin::Info& info)
      : Plugin(info)
   {
      /* Do nothing. */ ;
   }

public:
   static inf::PluginPtr create(const inf::plugin::Info& info);

   virtual ~ModelLoaderPlugin()
   {
      /* Do nothing. */ ;
   }

   virtual std::string getDescription();

   /** Initialize and configure the plugin.
    *
    */
   virtual PluginPtr init(inf::ViewerPtr viewer);

   virtual void update(inf::ViewerPtr viewer);
}; // ModelLoaderPlugin

}  // namespace inf

#endif
