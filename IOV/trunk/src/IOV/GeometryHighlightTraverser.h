// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_GEOMETRY_HIGHLIGHT_TRAVERSER_H_
#define _INF_GEOMETRY_HIGHLIGHT_TRAVERSER_H_

#include <IOV/Config.h>

#include <string>
#include <map>
#include <vector>
#include <boost/filesystem/path.hpp>
#include <boost/variant.hpp>

#include <OpenSG/OSGNode.h>
#include <OpenSG/OSGAction.h>
#include <OpenSG/OSGGeometry.h>
#include <OpenSG/OSGMaterial.h>
#include <OpenSG/OSGChunkMaterial.h>
#include <OpenSG/OSGSHLChunk.h>
#include <OpenSG/OSGMaterialGroup.h>

#include <IOV/Util/Exceptions.h>


namespace inf
{

/**
 * Allows sub-trees to have highlight materials applied to them.
 */
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

   /**
    *
    * @since 0.9.0
    */
   typedef boost::variant<bool, OSG::Int32, OSG::Real32, OSG::Vec2f,
                          OSG::Vec3f, OSG::Vec4f>
      uniform_variant_t;

   /**
    *
    * @since 0.9.0
    */
   typedef std::map<std::string, uniform_variant_t> uniform_map_t;

   GeometryHighlightTraverser();

   ~GeometryHighlightTraverser();

   void extendShaderSearchPath(const std::vector<boost::filesystem::path>& path);

   /**
    * Creates an OSG::ChunkMaterialPtr using the given information.  The
    * parameters \p vertexShaderFile and \p fragmentShaderFile name the files
    * containing vertex and fragment programs respectively.  The vector of
    * OSG::StateChunkRefPtr objects provides additional state chunks that are
    * added to the OSG::ChunkMaterialPtr that is created.  The map of uniform
    * parameters (mapping uniform parameter name to uniform parameter value)
    * is used to set any uniform parameters for the shader.  Finally, the
    * OSG::SHLChunkRefPtr, if provided, is used as the SHL state chunk for the
    * created OSG::ChunkMaterialPtr.  If \p shlChunk is not provided, one is
    * created and set up internally.
    *
    * @param vertexShaderFile   The name of the file containing the vertex
    *                           shader program.  If this is not an absolute
    *                           path and the file does not exist in the current
    *                           working directory, then the shader search path
    *                           will be used to find the file.
    * @param fragmentShaderFile The name of the file containing the fragment
    *                           shader program.  If this is not an absolute
    *                           path and the file does not exist in the current
    *                           working directory, then the shader search path
    *                           will be used to find the file.
    * @param chunks             A vector of state chunks that will be added to
    *                           the OSG::ChunkMaterialPtr object that is
    *                           created by this method.  This parameter is
    *                           optional.
    *
    * @return The unique identifier (with respect to this instance) for the
    *         newly created material.
    *
    * @throw inf::Exception is thrown if either the vertex or the fragment
    *        shader files cannot be read for whatever reason.
    *
    * @see extendShaderSearchPath
    *
    * @since 0.9.0
    */
   unsigned int createSHLMaterial(const std::string& vertexShaderFile,
                                  const std::string& fragmentShaderFile,
                                  const std::vector<OSG::StateChunkRefPtr>& chunks = std::vector<OSG::StateChunkRefPtr>())
   {
      uniform_map_t uniform_params;
      return createSHLMaterial(vertexShaderFile, fragmentShaderFile, chunks,
                               uniform_params);
   }

   /**
    * Creates an OSG::ChunkMaterialPtr using the given information.  The
    * parameters \p vertexShaderFile and \p fragmentShaderFile name the files
    * containing vertex and fragment programs respectively.  The vector of
    * OSG::StateChunkRefPtr objects provides additional state chunks that are
    * added to the OSG::ChunkMaterialPtr that is created.  The map of uniform
    * parameters (mapping uniform parameter name to uniform parameter value)
    * is used to set any uniform parameters for the shader.  Finally, the
    * OSG::SHLChunkRefPtr, if provided, is used as the SHL state chunk for the
    * created OSG::ChunkMaterialPtr.  If \p shlChunk is not provided, one is
    * created and set up internally.
    *
    * @param vertexShaderFile   The name of the file containing the vertex
    *                           shader program.  If this is not an absolute
    *                           path and the file does not exist in the current
    *                           working directory, then the shader search path
    *                           will be used to find the file.
    * @param fragmentShaderFile The name of the file containing the fragment
    *                           shader program.  If this is not an absolute
    *                           path and the file does not exist in the current
    *                           working directory, then the shader search path
    *                           will be used to find the file.
    * @param chunks             A vector of state chunks that will be added to
    *                           the OSG::ChunkMaterialPtr object that is
    *                           created by this method.
    * @param uniformParams      A map of uniform parameter names to uniform
    *                           parameter values.  The allowed value types are
    *                           those accepted by
    *                           OSG::SHLChunk::setUniformParameter().
    * @param shlChunk           The SHL state chunk to use for the material.
    *                           This is the state chunk to which the uniform
    *                           parameters will be applied.  This parameter is
    *                           optional.  If no value is given, then an
    *                           OSG::SHLChunkPtr will be created internally
    *                           and added to the OSG::ChunkMaterialPtr that
    *                           is created by this method.
    *
    * @return The unique identifier (with respect to this instance) for the
    *         newly created material.
    *
    * @throw inf::Exception is thrown if either the vertex or the fragment
    *        shader files cannot be read for whatever reason.
    *
    * @see extendShaderSearchPath
    *
    * @since 0.9.0
    */
   unsigned int createSHLMaterial(const std::string& vertexShaderFile,
                                  const std::string& fragmentShaderFile,
                                  const std::vector<OSG::StateChunkRefPtr>& chunks,
                                  uniform_map_t& uniformParams,
                                  OSG::SHLChunkRefPtr shlChunk = OSG::SHLChunkRefPtr());

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

   /**
    * Register material with list of materials that are highlighted.
    */
   unsigned int registerMaterial(OSG::MaterialRefPtr mat);

   /** Return the material with the given id. */
   OSG::MaterialRefPtr getHighlight(const unsigned int id);

   /** Return the number of materials current registered. */
   unsigned int getNumMaterials() const;

   /**
    * Returns if the specified material currently has a highlight
    * material.
    *
    * @param mat The material to test.
    */
   bool hasHighlight(OSG::MaterialRefPtr mat) const;

   /**
    * Adds the specified highlight material to all nodes under
    * the given node.
    * 
    * @param node The root of the sub-tree that you want to remove
    *             the material from.
    * @param id The material id of the highlight that you want to add.
    *
    * @throw inf::Exception is thrown if \p id is not a valid material ID.
    */
   void addHighlightMaterial(OSG::NodePtr node, const unsigned int id);

   /**
    * Swap the specified highlight materials for all nodes under
    * the given node.
    * 
    * @param node The root of the sub-tree that you want to remove
    *             the material from.
    * @param oldId The material id of the highlight that you want to remove.
    * @param newId The material id of the highlight that you want to add.
    *
    * @throw inf::Exception is thrown if \p oldId or \p newId is not a valid
    *        material ID.
    */
   void swapHighlightMaterial(OSG::NodePtr node, const unsigned int oldId,
                              const unsigned int newId);

   /**
    * Removes the specified highlight material from all nodes under
    * the given node.
    * 
    * @param node The root of the sub-tree that you want to remove
    *             the material from.
    * @param id The material id of the highlight that you want to remove.
    *
    * @throw inf::Exception is thrown if \p id is not a valid material ID.
    */
   void removeHighlightMaterial(OSG::NodePtr node, const unsigned int id);

private:
   /**
    * Create default scribing materials with different diffuse colors.
    */
   void createDefaultMaterials();

   /**
    * Clear the internal list of MaterialGroups and Geometry cores.
    */
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

   /**
    * Enter method that is called from the traverser to build
    * a list of all MaterialGroups and Geometry cores.
    */
   OSG::Action::ResultE enter(OSG::NodePtr& node);

   /**
    * The the absolute path to the specified shader file.
    *
    * @param filename The basename of the shader file.
    * @returns The absolute path the shader file.
    */
   boost::filesystem::path getCompleteShaderFile(const std::string& filename);

   /**
    * Determines if the specified material ID is valid.
    *
    * @param id The material ID to check.
    *
    * @throw inf::Exception is thrown if \p id is an invalid material ID.
    */
   void validateMaterialID(const unsigned int id);

   std::vector<OSG::MaterialGroupRefPtr> mMatGroupCores;   /**< MaterialGroups to change. */
   std::vector<OSG::GeometryRefPtr> mGeomCores;            /**< Geometry cores to change. */

   std::vector<boost::filesystem::path> mShaderSearchPath; /**< Path used to find named shader. */
   std::vector<OSG::MaterialRefPtr> mMaterials;            /**< Highlight materials that can be assigned. */
};

}


#endif
