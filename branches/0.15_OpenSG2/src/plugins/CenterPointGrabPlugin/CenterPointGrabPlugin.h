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

#ifndef _CENTER_POINT_GRAB_H_
#define _CENTER_POINT_GRAB_H_

#include <IOV/Plugin/PluginConfig.h>

#include <string>
#include <boost/enable_shared_from_this.hpp>

#include <OpenSG/OSGGeoProperties.h>
#include <OpenSG/OSGTypedGeoIntegralProperty.h>
#include <OpenSG/OSGSimpleGeometry.h>
#include <OpenSG/OSGSimpleMaterial.h>

#include <IOV/Plugin.h>
#include <IOV/WandInterfacePtr.h>
#include <IOV/Plugin/Buttons.h>


namespace inf
{

class CenterPointGrabPlugin
   : public inf::Plugin
   , public boost::enable_shared_from_this<CenterPointGrabPlugin>
{
public:
   static inf::PluginPtr create()
   {
      return inf::PluginPtr(new CenterPointGrabPlugin());
   }

   virtual ~CenterPointGrabPlugin()
   {
      /* Do nothing. */ ;
   }

   virtual std::string getDescription()
   {
      return std::string("Center Point Grabbing");
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

   CenterPointGrabPlugin()
      : GRAB_BUTTON(inf::buttons::GRAB_TOGGLE_BUTTON)
      , mIntersecting(false)
      , mGrabbing(false)
      , mIntersectColor(1.0f, 1.0f, 0.0f)
      , mGrabColor(1.0f, 0.0f, 1.0f)
   {
      /* Do nothing. */ ;
   }

private:
   void updateHighlight(OSG::NodePtr highlightNode);

   static std::string getElementType()
   {
      return std::string("center_point_grab_plugin");
   }

   const int GRAB_BUTTON;

   static const OSG::TransformNodePtr sEmptyCoredXformNode;

   inf::GrabDataPtr mGrabData;

   WandInterfacePtr mWandInterface;

   bool mIntersecting;
   bool mGrabbing;
   OSG::TransformNodePtr mIntersectedObj;
   OSG::TransformNodePtr mGrabbedObj;

   OSG::Color3f mIntersectColor;
   OSG::Color3f mGrabColor;

   OSG::GeometryNodePtr   mCoredHighlightNode;
   OSG::GeoPositions3fPtr mHighlightPoints;
   OSG::SimpleMaterialPtr mHighlightMaterial;
};

}

#endif
