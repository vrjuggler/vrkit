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

#ifndef _VRKIT_VIDEO_CAMERA_FBO_H_
#define _VRKIT_VIDEO_CAMERA_FBO_H_

#include <vrkit/Config.h>

#include <boost/enable_shared_from_this.hpp>

#include <OpenSG/OSGFBOViewport.h>

#include <vrkit/video/Camera.h>
#include <vrkit/video/CameraFBOPtr.h>


namespace vrkit
{

namespace video
{

/** \class CameraFBO CameraFBO.h vrkit/video/CameraFBO.h
 *
 * Renders a scene from a perspective camera into an FBO. This implementation
 * is based on OpenSG 1.8 FBO viewports.
 *
 * @note This class was renamed from vrkit::FboCamera and moved into the
 *       vrkit::video namespace in version 0.47.
 *
 * @since 0.37
 */
class VRKIT_CLASS_API CameraFBO : public Camera
{
protected:
   CameraFBO();

public:
   static CameraFBOPtr create();

   virtual ~CameraFBO();

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

}


#endif /* _VRKIT_VIDEO_CAMERA_FBO_H_ */
