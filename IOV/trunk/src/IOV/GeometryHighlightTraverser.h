// Copyright (C) Infiscape Corporation 2005

#ifndef _INF_GEOMETRY_HIGHLIGHT_TRAVERSER_H_
#define _INF_GEOMETRY_HIGHLIGHT_TRAVERSER_H_

#include <IOV/Config.h>

#include <string>
#include <vector>
#include <boost/filesystem/path.hpp>

#include <OpenSG/OSGNode.h>
#include <OpenSG/OSGAction.h>
#include <OpenSG/OSGGeometry.h>
#include <OpenSG/OSGMaterial.h>
#include <OpenSG/OSGChunkMaterial.h>
#include <OpenSG/OSGSHLChunk.h>

#include <IOV/Util/Exceptions.h>


namespace inf
{

class IOV_CLASS_API GeometryHighlightTraverser
{
public:
   /**
    * Identifiers for built-in highlight materials.
    */
   enum HighlightId
   {
      HIGHLIGHT0,    /**< OSG::SimpleMaterialPtr with scribing */
      HIGHLIGHT1,    /**< Another OSG::SimpleMaterialPtr with scribing and
                          a different diffuse color */
      LAST_HIGHLIGHT /**< Internal use only! Not a valid highlight ID! */
   };

   GeometryHighlightTraverser();

   ~GeometryHighlightTraverser();

   void extendShaderSearchPath(const std::vector<boost::filesystem::path>& path);

   unsigned int createSHLMaterial(const std::string& vertexShaderFile,
                                  const std::string& fragmentShaderFile)
      throw(inf::Exception);

   /**
    * Creates an OSG::ChunkMaterialPtr using the given OSG::SHLChunkPtr as
    * a starting point.  The primary purpose of this method is to hide the
    * details of loading the vertex and fragment programs and adding them to
    * the given SHL chunk.
    *
    * @since 0.7.1
    */
   unsigned int createSHLMaterial(const std::string& vertexShaderFile,
                                  const std::string& fragmentShaderFile,
                                  OSG::SHLChunkRefPtr shlChunk)
      throw(inf::Exception)
   {
      std::vector<OSG::StateChunkRefPtr> vec;
      return createSHLMaterial(vertexShaderFile, fragmentShaderFile, shlChunk,
                               vec);
   }

   /**
    * Creates an OSG::ChunkMaterialPtr using the given OSG::SHLChunkPtr as
    * a starting point.  The primary purpose of this method is to hide the
    * details of loading the vertex and fragment programs and adding them to
    * the given SHL chunk.  Finally, the vector of additional state chunks is
    * added to the newly created OSG::ChunkMaterialPtr.
    *
    * @since 0.7.1
    */
   unsigned int createSHLMaterial(const std::string& vertexShaderFile,
                                  const std::string& fragmentShaderFile,
                                  OSG::SHLChunkRefPtr shlChunk,
                                  const std::vector<OSG::StateChunkRefPtr>& extraChunks)
      throw(inf::Exception);

   /**
    * Creates and registers a very simple scribing material.
    *
    * @param isLit        Whether the new material will be lit.
    * @param frontMode    The front mode for the polygons (e.g., GL_LINE).
    * @param offsetLine   Enable/disable polygon line offset.
    * @param offsetFill   Enable/disable polygon fill offset.
    * @param offsetPoint  Enable/disable polygon point offset.
    * @param offsetFactor The polygon offset factor.
    * @param offsetBias   The poloygon offset bias.
    * @param diffuseColor The diffuse color for the new material.
    *
    * @return The unique ID for the newly created scribe material.
    *
    * @since 0.5.1
    */
   unsigned int createScribeMaterial(const bool isLit,
                                     const unsigned int frontMode,
                                     const bool offsetLine,
                                     const bool offsetFill,
                                     const bool offsetPoint,
                                     const float offsetFactor,
                                     const float offsetBias,
                                     const OSG::Color3f& diffuseColor);

   unsigned int registerMaterial(OSG::MaterialRefPtr mat);

   /** Return the material with the given id. */
   OSG::MaterialRefPtr getHighlight(const unsigned int id);

   /** Return the number of materials current registered. */
   unsigned int getNumMaterials() const;

   bool hasHighlight(OSG::MaterialRefPtr mat) const;

   void addHighlightMaterial(OSG::NodePtr node, const unsigned int id)
      throw(inf::Exception);

   void swapHighlightMaterial(OSG::NodePtr node, const unsigned int oldId,
                              const unsigned int newId)
      throw(inf::Exception);

   void removeHighlightMaterial(OSG::NodePtr node, const unsigned int id)
      throw(inf::Exception);

private:
   void createDefaultMaterials();

   void reset();

   /**
    * Performs a new traversal rooted at the given node and stores
    * the necessary information for later use with material
    * applications.
    *
    * @see addHighlightMaterial()
    * @see changeHighlightMaterial()
    * @see removeHighlightMaterial()
    */
   void traverse(OSG::NodePtr node);

   OSG::Action::ResultE enter(OSG::NodePtr& node);

   boost::filesystem::path getCompleteShaderFile(const std::string& filename);

   void validateMaterialID(const unsigned int id) throw(inf::Exception);

private:
   std::vector<OSG::GeometryRefPtr> mGeomCores;

   std::vector<boost::filesystem::path> mShaderSearchPath;
   std::vector<OSG::MaterialRefPtr> mMaterials;

};

}


#endif
