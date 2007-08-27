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

#ifndef _VRKIT_MODEL_LOADER_PLUGIN_H_
#define _VRKIT_MODEL_LOADER_PLUGIN_H_

#include <vrkit/plugin/Config.h>

#include <string>
#include <boost/enable_shared_from_this.hpp>

#include <vrkit/viewer/Plugin.h>


namespace vrkit
{

class ModelLoaderPlugin
   : public viewer::Plugin
   , public boost::enable_shared_from_this<ModelLoaderPlugin>
{
protected:
   ModelLoaderPlugin(const plugin::Info& info)
      : viewer::Plugin(info)
   {
      /* Do nothing. */ ;
   }

public:
   static viewer::PluginPtr create(const plugin::Info& info);

   virtual ~ModelLoaderPlugin()
   {
      /* Do nothing. */ ;
   }

   virtual std::string getDescription();

   /** Initialize and configure the plugin.
    *
    */
   virtual viewer::PluginPtr init(ViewerPtr viewer);

   virtual void update(ViewerPtr viewer);
}; // ModelLoaderPlugin

}  // namespace vrkit


#endif /* _VRKIT_MODEL_LOADER_PLUGIN_H_ */
