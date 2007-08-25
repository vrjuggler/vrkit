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

#ifndef _INF_VOLUME_DRAWING_PLUGIN_H_
#define _INF_VOLUME_DRAWING_PLUGIN_H_

#include <IOV/Plugin/PluginConfig.h>

#include <boost/enable_shared_from_this.hpp>

#include <IOV/Plugin.h>
#include <IOV/ViewerPtr.h>


namespace inf
{

class VolumeDrawingPlugin
   : public inf::Plugin
   , public boost::enable_shared_from_this<VolumeDrawingPlugin>
{
protected:
   VolumeDrawingPlugin(const inf::plugin::Info& info);

public:
   static inf::PluginPtr create(const inf::plugin::Info& info)
   {
      return inf::PluginPtr(new VolumeDrawingPlugin(info));
   }

   virtual ~VolumeDrawingPlugin()
   {
      /* Do nothing. */ ;
   }

   virtual std::string getDescription()
   {
      return std::string("Volume Drawing");
   }

   /**
    * Initializes this plug-in. Currently, there is nothing to initialize.
    *
    * @param viewer The VR Juggler application object.
    *
    * @return A pointer to this plug-in is returned.
    */
   virtual PluginPtr init(inf::ViewerPtr viewer);

   /**
    * Activates OpenSG volume drawing for the newly opened context.
    *
    * @post The context-specific render action for this context will render
    *       node bounding volumes.
    *
    * @param viewer The VR Juggler application object.
    */
   virtual void contextInit(inf::ViewerPtr viewer);

   /**
    * Does nothing as this plug-in does not have any state.
    *
    * @param viewer The VR Juggler application object.
    */
   virtual void update(inf::ViewerPtr viewer);
};

}


#endif /* _INF_VOLUME_DRAWING_PLUGIN_H_ */
