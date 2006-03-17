// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_GRAB_PLUGIN_H_
#define _INF_GRAB_PLUGIN_H_

#include <IOV/Plugin/PluginConfig.h>

#include <string>
#include <map>
#include <vector>
#include <boost/enable_shared_from_this.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/function.hpp>
#include <gmtl/Matrix.h>

#include <snx/SoundHandle.h>

#include <IOV/Plugin.h>
#include <IOV/WandInterfacePtr.h>
#include <IOV/GeometryHighlightTraverser.h>
#include <IOV/PluginFactory.h>
#include <IOV/Util/DigitalCommand.h>

#include "IntersectionStrategyPtr.h"
#include "MoveStrategyPtr.h"
#include "IntersectionStrategy.h"
#include "MoveStrategy.h"


namespace inf
{

class GrabPlugin
   : public inf::Plugin
   , public boost::enable_shared_from_this<GrabPlugin>
{
public:
   static inf::PluginPtr create()
   {
      return inf::PluginPtr(new GrabPlugin());
   }

   virtual ~GrabPlugin()
   {
      /* Do nothing. */ ;
   }

   virtual std::string getDescription()
   {
      return std::string("Grabbing");
   }

   virtual void init(inf::ViewerPtr viewer);

   virtual void updateState(inf::ViewerPtr viewer);

   virtual void run(inf::ViewerPtr viewer);

   bool config(jccl::ConfigElementPtr elt);

   /**
    * Invokes the global scope delete operator.  This is required for proper
    * releasing of memory in DLLs on Win32.
    */
   void operator delete(void* p)
   {
      ::operator delete(p);
   }

protected:
   /**
    * Deletes this object.  This is an implementation of the pure virtual
    * inf::Plugin::destroy() method.
    */
   virtual void destroy()
   {
      delete this;
   }

   GrabPlugin();

   void focusChanged(inf::ViewerPtr viewer);

private:
   static std::string getElementType()
   {
      return std::string("iov_grab_plugin");
   }

   std::vector<boost::filesystem::path> mShaderSearchPath;
   bool mEnableShaders;
   std::string mIsectVertexShaderFile;
   std::string mIsectFragmentShaderFile;
   float       mIsectUniformScale;
   float       mIsectUniformExponent;
   std::string mGrabVertexShaderFile;
   std::string mGrabFragmentShaderFile;
   float       mGrabUniformScale;
   float       mGrabUniformExponent;

   WandInterfacePtr mWandInterface;

   /** Button for grabbing and releasing objects. */
   inf::DigitalCommand mGrabBtn;
   std::string mGrabText;

   bool mIntersecting;
   bool mGrabbing;
   OSG::TransformNodePtr mIntersectedObj;

   GeometryHighlightTraverser mGeomTraverser;
   unsigned int mIsectHighlightID;
   unsigned int mGrabHighlightID;
   OSG::Color3f mIntersectColor;
   OSG::Color3f mGrabColor;

   snx::SoundHandle mIntersectSound;
   snx::SoundHandle mGrabSound;

   std::vector<std::string> mStrategyPluginPath;

   inf::PluginFactoryPtr mPluginFactory;

   /** @name Intersection Strategy */
   //@{
   std::map< std::string, boost::function<IntersectionStrategyPtr ()> > mIsectStrategyMap;
   IntersectionStrategyPtr mIsectStrategy;
   std::string mIsectStrategyName;
   //@}

   /** @name Move Strategy */
   //@{
   gmtl::Matrix44f mGrabbed_pobj_M_obj;
   std::map< std::string, boost::function<MoveStrategyPtr ()> > mMoveStrategyMap;
   std::vector<MoveStrategyPtr> mMoveStrategies;
   std::vector<std::string> mMoveStrategyNames;
   //@}
};

}


#endif
