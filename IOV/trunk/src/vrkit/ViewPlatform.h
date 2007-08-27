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

#ifndef _VRKIT_VIEW_PLATFORM_H_
#define _VRKIT_VIEW_PLATFORM_H_

#include <boost/signal.hpp>

#include <gmtl/Matrix.h>
#include <gmtl/MatrixOps.h>

#include <vrkit/ViewerPtr.h>
#include <vrkit/signal/Proxy.h>


namespace vrkit
{

/**
 * This class holds the position of the virtual environment in the virtual
 * world. It is the sort of "magic carpet" that the user is moving around on.
 * The view platform corresponds to the physical display system that the
 * user is inside or using.
 *
 * See vrkit::Scene for a description of the coordinate systems used.
 *
 * @see vrkit::Scene
 */
class VRKIT_CLASS_API ViewPlatform
{
public:
   ViewPlatform();

   /** Updates the view platform (navigation). */
   void update(ViewerPtr viewer);

   /**
    * Returns the current position.
    * This is the position of the view platform in the virtual world: vw_M_vp.
    */
   const gmtl::Matrix44f& getCurPos() const
   {
      return mCurPos;
   }

   /**
    * Sets the current position of the view platform to the given
    * transformation and emits the "platform moved" signal indicating this
    * fact to connected slots.
    *
    * @post \c mCurPos == \p mat and \c mCurPosInv is the inverse of the
    *       updated \c mCurPos.
    *
    * @param mat The new transformation matrix for the view platform.
    */
   void setCurPos(const gmtl::Matrix44f& mat)
   {
      mCurPos = mat;
      gmtl::invert(mCurPosInv, mCurPos);
      mPlatformMoved();
   }

   /**
    * Returns the inverse of the current position: vp_M_vw.
    */
   const gmtl::Matrix44f& getCurPosInv() const
   {
      return mCurPosInv;
   }

   /** @name Signals */
   //@{
   typedef boost::signal<void ()> signal_t;

   /**
    * Provides access to the "platform moved" signal for this view platform
    * so that slots may be connected to it.
    *
    * @since 0.26.7
    */
   signal::Proxy<signal_t> platformMoved()
   {
      return signal::Proxy<signal_t>(mPlatformMoved);
   }
   //@}

private:
   signal_t mPlatformMoved;

   /**
    * The current positon of the platform in the virtual world: vw_M_vp.
    * This is the positiong of the virtual platform in the virtual world.
    */
   gmtl::Matrix44f   mCurPos;

   /**
    * Inverse matrix of the current position: vp_M_vw.
    */
   gmtl::Matrix44f   mCurPosInv;
};

}


#endif /* _VRKIT_VIEW_PLATFORM_H_ */
