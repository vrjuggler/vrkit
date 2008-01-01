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

#ifndef _VRKIT_MODEL_SWAP_PLUGIN_H_
#define _VRKIT_MODEL_SWAP_PLUGIN_H_

#include <vrkit/plugin/Config.h>

#include <string>
#include <boost/enable_shared_from_this.hpp>

#include <OpenSG/OSGSwitch.h>

#include <vrkit/WandInterfacePtr.h>
#include <vrkit/viewer/Plugin.h>
#include <vrkit/util/DigitalCommand.h>


namespace vrkit
{

class ModelSwapPlugin
   : public viewer::Plugin
   , public boost::enable_shared_from_this<ModelSwapPlugin>
{
protected:
   ModelSwapPlugin(const plugin::Info& info)
      : viewer::Plugin(info)
   {
      /* Do nothing. */ ;
   }

public:
   static viewer::PluginPtr create(const plugin::Info& info);

   virtual ~ModelSwapPlugin()
   {
      /* Do nothing. */ ;
   }

   virtual std::string getDescription();

   /**
    * Initializes and configures the plug-in.
    */
   virtual viewer::PluginPtr init(ViewerPtr viewer);

   virtual void update(ViewerPtr viewer);

protected:
   WandInterfacePtr     mWandInterface; /**< Ptr to the wand interface to use. */
   util::DigitalCommand mSwapButton;    /**< Ptr to the digital button. */
   OSG::NodeRefPtr      mSwitchNode;
   OSG::SwitchRefPtr    mSwitchCore;
}; // ModelSwapPlugin

}  // namespace vrkit


#endif /* _VRKIT_MODEL_SWAP_PLUGIN_H_ */
