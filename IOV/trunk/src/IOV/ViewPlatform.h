// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_VIEW_PLATFORM_H_
#define _INF_VIEW_PLATFORM_H_

#include <boost/signal.hpp>

#include <gmtl/Matrix.h>
#include <gmtl/MatrixOps.h>

#include <IOV/ViewerPtr.h>
#include <IOV/Util/SignalProxy.h>


namespace inf
{

/**
 * This class holds the position of the virtual environment in the virtual
 * world. It is the sort of "magic carpet" that the user is moving around on.
 * The view platform corresponds to the physical display system that the
 * user is inside or using.
 *
 * See inf::Scene for a description of the coordinate systems used.
 *
 * @see inf::Scene
 */
class IOV_CLASS_API ViewPlatform
{
public:
   ViewPlatform()
   {;}

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
   void setCurPos(const gmtl::Matrix44f& mat);

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
   inf::SignalProxy<signal_t> platformMoved()
   {
      return inf::SignalProxy<signal_t>(mPlatformMoved);
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


#endif
