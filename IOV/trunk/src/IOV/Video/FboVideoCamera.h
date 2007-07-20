// Copyright (C) Infiscape Corporation 2005-2007

#ifndef _INF_FBO_VIDEO_CAMERA_H_
#define _INF_FBO_VIDEO_CAMERA_H_

#include <IOV/Config.h>

#include <boost/enable_shared_from_this.hpp>

#include <OpenSG/OSGViewport.h>
#include <OpenSG/OSGTransform.h>
#include <OpenSG/OSGTextureChunk.h>
#include <OpenSG/OSGWindow.h>
#include <OpenSG/OSGFBOViewport.h>

#include <IOV/Video/FboVideoCameraPtr.h>

namespace inf
{

/**
 * Renders a scene from a perspective camera into a FBO.
 *
 * @since 0.37
 */
class IOV_CLASS_API FboVideoCamera : public boost::enable_shared_from_this<FboVideoCamera>
{
protected:
   FboVideoCamera()
      : mFboVP(OSG::NullFC)
      , mFboCamBeacon(OSG::NullFC)
      , mFboCamTrans(OSG::NullFC)
      , mFboTexture(OSG::NullFC)
      , mFboWidth(512)
      , mFboHeight(512)
   {;}

public:
   static FboVideoCameraPtr create();

   virtual ~FboVideoCamera();

   /**
    * Initialize the FBO camera.
    */
   FboVideoCameraPtr init();
   void contextInit(OSG::WindowPtr gwin);

   void setupFBO();

   OSG::FBOViewportPtr getFBO()
   {
      return mFboVP;
   }

   OSG::TextureChunkPtr getTexture()
   {
      return mFboTexture;
   }

   void setCameraPos(OSG::Matrix camPos)
   {
      // set the fbo camera to show the whole model
      OSG::beginEditCP(mFboCamTrans, OSG::Transform::MatrixFieldMask);
         mFboCamTrans->setMatrix(camPos);
      OSG::endEditCP(mFboCamTrans, OSG::Transform::MatrixFieldMask);
   }

   OSG::NodePtr getDebugPlane();

private:
   OSG::FBOViewportPtr     mFboVP;
   OSG::NodePtr            mFboCamBeacon;
   OSG::RefPtr<OSG::TransformPtr>       mFboCamTrans;
   OSG::TextureChunkPtr    mFboTexture;
   // fbo size
   const int               mFboWidth;
   const int               mFboHeight;

};

}

#endif /*_INF_FBO_VIDEO_CAMERA_H_*/
