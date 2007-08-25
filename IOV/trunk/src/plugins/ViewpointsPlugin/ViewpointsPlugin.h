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

#ifndef _INF_VIEWPOINTS_PLUGIN_H_
#define _INF_VIEWPOINTS_PLUGIN_H_

#include <IOV/Plugin/PluginConfig.h>

#include <string>
#include <boost/enable_shared_from_this.hpp>
#include <gmtl/Matrix.h>
#include <vector>

#include <IOV/Plugin.h>
#include <IOV/WandInterfacePtr.h>
#include <IOV/Util/DigitalCommand.h>


namespace inf
{

/** Plugin for moving between preset viewpoints.
 *
 */
class ViewpointsPlugin
   : public inf::Plugin
   , public boost::enable_shared_from_this<ViewpointsPlugin>
{
protected:
   ViewpointsPlugin(const inf::plugin::Info& info)
      : Plugin(info)
      , mNextViewpoint(0)
   {;}

public:
   static PluginPtr create(const inf::plugin::Info& info);

   virtual ~ViewpointsPlugin()
   {;}

   virtual std::string getDescription()
   {
      return std::string("Viewpoints Plugin");
   }

   virtual PluginPtr init(inf::ViewerPtr viewer);

   virtual void update(inf::ViewerPtr viewer);

protected:
   struct Viewpoint
   {
      gmtl::Matrix44f   mXform;
      std::string       mName;
   };

private:
   WandInterfacePtr        mWandInterface;      /**< Ptr to the wand interface to use. */
   inf::DigitalCommand     mControlCmd;         /**< The control digital command. */
   unsigned                mNextViewpoint;      /**< The next viewpoint to switch to. */
   std::vector<Viewpoint>  mViewpoints;         /**< The predefined viewpoints. */
};

} // namespace inf


#endif
