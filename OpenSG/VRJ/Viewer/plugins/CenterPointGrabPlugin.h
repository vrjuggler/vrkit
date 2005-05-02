#ifndef _CENTER_POINT_GRAB_H_
#define _CENTER_POINT_GRAB_H_

#include <OpenSG/VRJ/Viewer/plugins/PluginConfig.h>

#include <string>
#include <boost/enable_shared_from_this.hpp>

#include <OpenSG/OSGGeoPropPtrs.h>
#include <OpenSG/OSGSimpleGeometry.h>
#include <OpenSG/OSGSimpleMaterial.h>

#include <OpenSG/VRJ/Viewer/IOV/Plugin.h>
#include <OpenSG/VRJ/Viewer/IOV/WandInterfacePtr.h>
#include <OpenSG/VRJ/Viewer/plugins/CenterPointGrabPluginPtr.h>


namespace inf
{

typedef OSG::CoredNodePtr<OSG::Geometry> CoredGeomPtr;

class CenterPointGrabPlugin
   : public inf::Plugin
   , public boost::enable_shared_from_this<CenterPointGrabPlugin>
{
public:
   static CenterPointGrabPluginPtr create()
   {
      return CenterPointGrabPluginPtr(new CenterPointGrabPlugin);
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

   virtual void update(inf::ViewerPtr viewer);

   virtual bool canHandleElement(jccl::ConfigElementPtr elt);

   virtual bool config(jccl::ConfigElementPtr elt);

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
      : GRAB_BUTTON(0)
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
};

}

#endif
