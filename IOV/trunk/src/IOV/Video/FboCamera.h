// Copyright (C) Infiscape Corporation 2005-2007

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

   void setSize(osg::UInt32, osg::UInt32);

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
