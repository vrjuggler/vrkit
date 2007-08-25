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

#ifndef SIMPLE_NAV_PLUGIN_H
#define SIMPLE_NAV_PLUGIN_H

#include <IOV/Plugin/PluginConfig.h>

#include <boost/enable_shared_from_this.hpp>

#include <IOV/WandInterfacePtr.h>
#include <IOV/Plugin.h>
#include <IOV/Plugin/NavPlugin.h>
#include <IOV/Util/DigitalCommand.h>


namespace inf
{

class SimpleNavPlugin
   : public NavPlugin
   , public boost::enable_shared_from_this<SimpleNavPlugin>
{
protected:
   SimpleNavPlugin(const inf::plugin::Info& info);

public:
   static inf::PluginPtr create(const inf::plugin::Info& info)
   {
      return inf::PluginPtr(new SimpleNavPlugin(info));
   }

   virtual ~SimpleNavPlugin()
   {
      ;
   }

   virtual std::string getDescription()
   {
      return std::string("Navigation");
   }

   virtual PluginPtr init(ViewerPtr viewer);

protected:
   enum NavState
   {
      RESET,
      TRANSLATE,
      ROTATE
   };

   /** Navigation mode. */
   enum NavMode
   {
      WALK,     /**< Walk (drive) mode */
      FLY       /**< Fly mode */
   };

   virtual void focusChanged(inf::ViewerPtr viewer);

   virtual void updateNav(ViewerPtr viewer, ViewPlatform& viewPlatform);

   static std::string getElementType()
   {
      return std::string("simple_nav_plugin");
   }

   WandInterfacePtr mWandInterface;

   bool mCanNavigate;

   float mVelocity;
   NavMode mNavMode;

   inf::DigitalCommand mForBtn;         /**< Button for forward motion. */
   inf::DigitalCommand mRevBtn;         /**< Button for reverse. */
   inf::DigitalCommand mRotateBtn;      /**< Button for rotate. */
   inf::DigitalCommand mModeBtn;        /**< Button for swapping mode. */

   std::string mForwardText;
   std::string mReverseText;
   std::string mRotateText;
   std::string mModeText;
};

}


#endif
