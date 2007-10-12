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

#ifndef _VRKIT_VIEWPOINTS_PLUGIN_H_
#define _VRKIT_VIEWPOINTS_PLUGIN_H_

#include <vrkit/plugin/Config.h>

#include <string>
#include <vector>
#include <boost/enable_shared_from_this.hpp>
#include <gmtl/Matrix.h>

#include <vrkit/WandInterfacePtr.h>
#include <vrkit/viewer/Plugin.h>
#include <vrkit/util/DigitalCommand.h>


namespace vrkit
{

/**
 * Plug-in for moving between configured viewpoints.
 */
class ViewpointsPlugin
   : public viewer::Plugin
   , public boost::enable_shared_from_this<ViewpointsPlugin>
{
protected:
   ViewpointsPlugin(const plugin::Info& info)
      : viewer::Plugin(info)
      , mNextViewpoint(0)
   {;}

public:
   static viewer::PluginPtr create(const plugin::Info& info);

   virtual ~ViewpointsPlugin()
   {;}

   virtual std::string getDescription()
   {
      return std::string("Viewpoints Plug-in");
   }

   virtual viewer::PluginPtr init(ViewerPtr viewer);

   virtual void update(ViewerPtr viewer);

protected:
   struct Viewpoint
   {
      gmtl::Matrix44f   mXform;
      std::string       mName;
   };

private:
   WandInterfacePtr        mWandInterface;      /**< Ptr to the wand interface to use. */
   util::DigitalCommand    mControlCmd;         /**< The control digital command. */
   unsigned int            mNextViewpoint;      /**< The next viewpoint to switch to. */
   std::vector<Viewpoint>  mViewpoints;         /**< The predefined viewpoints. */
};

} // namespace vrkit


#endif /* _VRKIT_VIEWPOINTS_PLUGIN_H_ */
