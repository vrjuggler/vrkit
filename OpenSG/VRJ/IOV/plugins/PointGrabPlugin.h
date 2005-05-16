#ifndef _INF_POINT_GRAB_PLUGIN_H_
#define _INF_POINT_GRAB_PLUGIN_H_

#include <OpenSG/VRJ/Viewer/plugins/PluginConfig.h>

#include <string>
#include <boost/enable_shared_from_this.hpp>
#include <gmtl/Matrix.h>

#include <OpenSG/OSGGeoPropPtrs.h>
#include <OpenSG/OSGSimpleGeometry.h>
#include <OpenSG/OSGSimpleMaterial.h>

#include <snx/SoundHandle.h>

#include <OpenSG/VRJ/Viewer/IOV/Plugin.h>
#include <OpenSG/VRJ/Viewer/IOV/WandInterfacePtr.h>
#include <OpenSG/VRJ/Viewer/plugins/Buttons.h>


namespace inf
{

typedef OSG::CoredNodePtr<OSG::Geometry> CoredGeomPtr;

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

   PointGrabPlugin()
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
      return std::string("point_grab_plugin");
   }

   const int GRAB_BUTTON;

   static const inf::CoredTransformPtr sEmptyCoredXformNode;

   inf::GrabDataPtr mGrabData;

   WandInterfacePtr mWandInterface;

   bool mIntersecting;
   bool mGrabbing;
   inf::CoredTransformPtr mIntersectedObj;
   inf::CoredTransformPtr mGrabbedObj;

   OSG::Color3f mIntersectColor;
   OSG::Color3f mGrabColor;

   inf::CoredGeomPtr      mCoredHighlightNode;
   OSG::GeoPositions3fPtr mHighlightPoints;
   OSG::SimpleMaterialPtr mHighlightMaterial;

   gmtl::Matrix44f m_wand_M_obj;

   snx::SoundHandle mIntersectSound;
   snx::SoundHandle mGrabSound;
};

}

#endif
