#ifndef _INF_POINT_GRAB_PLUGIN_H_
#define _INF_POINT_GRAB_PLUGIN_H_

#include <IOV/Plugin/PluginConfig.h>

#include <string>
#include <exception>
#include <boost/enable_shared_from_this.hpp>
#include <boost/filesystem/path.hpp>
#include <gmtl/Matrix.h>

#include <OpenSG/OSGGeoPropPtrs.h>
#include <OpenSG/OSGSimpleGeometry.h>
#include <OpenSG/OSGSimpleMaterial.h>
#include <OpenSG/OSGMaterialGroup.h>

#include <snx/SoundHandle.h>

#include <IOV/Plugin.h>
#include <IOV/WandInterfacePtr.h>


namespace inf
{

typedef OSG::CoredNodePtr<OSG::Geometry>      CoredGeomPtr;
typedef OSG::CoredNodePtr<OSG::MaterialGroup> CoredMatGroupPtr;

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

private:
   void updateHighlight(OSG::NodePtr highlightNode);

   OSG::RefPtr<OSG::ChunkMaterialPtr> createShader(const std::string& vertexShader,
                                                   const std::string& fragmentShader)
      throw(std::exception);

   boost::filesystem::path getCompleteShaderFile(const std::string& filename);

   static std::string getElementType()
   {
      return std::string("point_grab_plugin");
   }

   const int GRAB_BUTTON;

   std::vector<boost::filesystem::path> mShaderSearchPath;
   bool mEnableShaders;
   std::string mIsectVertexShaderFile;
   std::string mIsectFragmentShaderFile;
   std::string mGrabVertexShaderFile;
   std::string mGrabFragmentShaderFile;

   inf::GrabDataPtr mGrabData;

   WandInterfacePtr mWandInterface;

   bool mIntersecting;
   bool mGrabbing;
   inf::CoredTransformPtr mIntersectedObj;

   OSG::Color3f mIntersectColor;
   OSG::Color3f mGrabColor;

   bool mUsingShader;
   inf::CoredMatGroupPtr              mCoredHighlightNode;
   OSG::GeoPositions3fPtr             mHighlightPoints;
   OSG::RefPtr<OSG::ChunkMaterialPtr> mIsectHighlightMaterial;
   OSG::RefPtr<OSG::ChunkMaterialPtr> mGrabHighlightMaterial;

   gmtl::Matrix44f m_wand_M_obj;

   snx::SoundHandle mIntersectSound;
   snx::SoundHandle mGrabSound;
};

}

#endif
