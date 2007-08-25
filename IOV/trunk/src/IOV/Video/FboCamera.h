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

#ifndef _INF_FBO_CAMERA_H_
#define _INF_FBO_CAMERA_H_

#include <IOV/Config.h>

#include <boost/enable_shared_from_this.hpp>

#include <IOV/Video/Camera.h>
#include <IOV/Video/CameraPtr.h>
#include <IOV/Video/FboCameraPtr.h>

#include <OpenSG/OSGFBOViewport.h>

namespace inf
{

/**
 * Renders a scene from a perspective camera into a FBO.
 *
 * @since 0.37
 */
class IOV_CLASS_API FboCamera : public inf::Camera
{
protected:
   FboCamera();

public:
   static FboCameraPtr create();

   virtual ~FboCamera();

   void setWindow(OSG::WindowPtr window);

   CameraPtr init();

   void setSize(const osg::UInt32 width, const osg::UInt32 height);

   void setSceneRoot(OSG::NodePtr root);

   void render(osg::RenderAction*);

   void setTravMask(const OSG::UInt32 value);

   /**
    * Returns the FBOViewport.
    */
   OSG::FBOViewportPtr getFboViewport() const
   {
      return mFboVP;
   }

private:
   OSG::FBOViewportPtr                  mFboVP;         /**< FBOViewport that we use to render to an FBO. */
};

}

#endif /*_INF_FBO_CAMERA_H_*/
