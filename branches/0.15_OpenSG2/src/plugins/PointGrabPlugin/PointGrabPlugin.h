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

#ifndef _INF_POINT_GRAB_PLUGIN_H_
#define _INF_POINT_GRAB_PLUGIN_H_

#include <IOV/Plugin/PluginConfig.h>

#include <string>
#include <vector>
#include <boost/enable_shared_from_this.hpp>
#include <boost/filesystem/path.hpp>
#include <gmtl/Matrix.h>

#include <snx/SoundHandle.h>

#include <IOV/Plugin.h>
#include <IOV/WandInterfacePtr.h>
#include <IOV/GeometryHighlightTraverser.h>
#include <IOV/Util/DigitalCommand.h>


namespace inf
{

class PointGrabPlugin
   : public inf::Plugin
   , public boost::enable_shared_from_this<PointGrabPlugin>
{
public:
   static inf::PluginPtr create()
   {
      return inf::PluginPtr(new PointGrabPlugin());
   }

   virtual ~PointGrabPlugin()
   {
      /* Do nothing. */ ;
   }

   virtual std::string getDescription()
   {
      return std::string("Point Grabbing");
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

   PointGrabPlugin();

   void focusChanged(inf::ViewerPtr viewer);

private:
   static std::string getElementType()
   {
      return std::string("point_grab_plugin");
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

   inf::GrabDataPtr mGrabData;

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

   gmtl::Matrix44f m_wand_M_obj_xform;

   snx::SoundHandle mIntersectSound;
   snx::SoundHandle mGrabSound;
};

}


#endif