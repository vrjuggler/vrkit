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

#ifndef _VRKIT_WAND_NAV_PLUGIN_H_
#define _VRKIT_WAND_NAV_PLUGIN_H_

#include <vrkit/plugin/Config.h>

#include <boost/enable_shared_from_this.hpp>

#include <vpr/Util/Interval.h>

#include <vrkit/WandInterfacePtr.h>
#include <vrkit/util/DigitalCommand.h>
#include <vrkit/nav/Strategy.h>


namespace vrkit
{

class WandNavPlugin
   : public nav::Strategy
   , public boost::enable_shared_from_this<WandNavPlugin>
{
protected:
   WandNavPlugin(const plugin::Info& info);

public:
   static viewer::PluginPtr create(const plugin::Info& info)
   {
      return viewer::PluginPtr(new WandNavPlugin(info));
   }

   virtual ~WandNavPlugin()
   {
      ;
   }

   virtual std::string getDescription()
   {
      return std::string("Navigation");
   }

   virtual viewer::PluginPtr init(ViewerPtr viewer);

   /**
    * @pre \c canHandleElement() returned true.
    */
   bool config(jccl::ConfigElementPtr elt);

   /** @name Configuration methods */
   //@{
   void setMaximumVelocity(const float minVelocity);
   void setAcceleration(const float acceleration);
   void setDeceleration(const float deceleration);
   //@}

protected:
   enum NavState
   {
      NONE,
      RESET,
      TRANSLATE,
      ROTATE
   };

   /** Navigation mode. */
   enum NavMode
   {
      WALK = 0,     /**< Walk (drive) mode */
      FLY  = 1      /**< Fly mode */
   };

   virtual void focusChanged(ViewerPtr viewer);

   virtual void updateNav(ViewerPtr viewer, ViewPlatform& viewPlatform);

   static std::string getElementType()
   {
      return std::string("wand_nav_plugin");
   }

   vpr::Interval mLastFrameTime;

   WandInterfacePtr mWandInterface;

   bool mCanNavigate;

   float mVelocity;
   float mMaxVelocity;
   float mAcceleration;
   float mDeceleration;
   bool mIsDecelerationEnabled;
   float mRotationSensitivity;      /**< Scalar on rotation delta.  Used to adjust sensitivity. */
   NavMode mNavMode;

   util::DigitalCommand mForwardBtn;     /**< Button for forward motion. */
   util::DigitalCommand mReverseBtn;     /**< Button for reverse. */
   util::DigitalCommand mRotateBtn;      /**< Button for rotate. */
   util::DigitalCommand mModeBtn;        /**< Button for swapping mode. */
   util::DigitalCommand mResetBtn;       /**< Button for resetting location. */

   std::string mForwardText;
   std::string mReverseText;
   std::string mRotateText;
   std::string mModeText;
   std::string mResetText;
};

}


#endif /* _VRKIT_WAND_NAV_PLUGIN_H_ */
