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

#ifndef _VRKIT_VOLUME_DRAWING_PLUGIN_H_
#define _VRKIT_VOLUME_DRAWING_PLUGIN_H_

#include <vrkit/plugin/Config.h>

#include <boost/enable_shared_from_this.hpp>

#include <vrkit/ViewerPtr.h>
#include <vrkit/viewer/Plugin.h>


namespace vrkit
{

/**
 * This plug-in enables OpenSG rendering of node (box) volumes. This is not to
 * be confused with volume rendering.
 */
class VolumeDrawingPlugin
   : public viewer::Plugin
   , public boost::enable_shared_from_this<VolumeDrawingPlugin>
{
protected:
   VolumeDrawingPlugin(const plugin::Info& info);

public:
   static viewer::PluginPtr create(const plugin::Info& info)
   {
      return viewer::PluginPtr(new VolumeDrawingPlugin(info));
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
   virtual viewer::PluginPtr init(ViewerPtr viewer);

   /**
    * Activates OpenSG volume drawing for the newly opened context.
    *
    * @post The context-specific render action for this context will render
    *       node bounding volumes.
    *
    * @param viewer The VR Juggler application object.
    */
   virtual void contextInit(ViewerPtr viewer);

   /**
    * Does nothing as this plug-in does not have any state.
    *
    * @param viewer The VR Juggler application object.
    */
   virtual void update(ViewerPtr viewer);
};

}


#endif /* _VRKIT_VOLUME_DRAWING_PLUGIN_H_ */
