// vrkit is (C) Copyright 2005-2011
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

#ifndef _VRKIT_UTIL_GEOMETRY_HIGHLIGHT_TRAVERSER_H_
#define _VRKIT_UTIL_GEOMETRY_HIGHLIGHT_TRAVERSER_H_

#include <vrkit/Config.h>

#include <string>
#include <map>
#include <vector>
#include <boost/function.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/variant.hpp>

#include <OpenSG/OSGNode.h>
#include <OpenSG/OSGAction.h>
#include <OpenSG/OSGGeometry.h>
#include <OpenSG/OSGMaterial.h>
#include <OpenSG/OSGChunkMaterial.h>
#include <OpenSG/OSGSHLChunk.h>
#include <OpenSG/OSGMaterialGroup.h>


namespace vrkit
{

namespace util
{

/** \class HighlightCoreFinder HighlightCoreFinder.h vrkit/util/HighlightCoreFinder.h
 *
 * Helper class used by vrkit::util::GeometryHighlightTraverser to find node
 * cores that support the use of materials. Instances of this class can be
 * used by external code to pre-traverse a sub-tree of the scene graph in
 * order to find such node cores so that vrkit::GeometryHighlightTraverser
 * does not have to do so itself. For scene graph nodes with a static
 * structure, this can avoid making repeated (unnecessary) traversals.
 *
 * The current set node core types that are found by this traverser is as
 * follows:
 *    - OSG::Geometry
 *    - OSG::MaterialGroup
 *
 * @note Instances of this class retain a list of node core pointers after
 *       traversal. Care must be taken to ensure that traversal state is
 *       managed correctly if instances of this class are retained.
 *
 * @see vrkit::util::GeometryHighlightTraverser
 *
 * @note This class was moved into the vrkit::util namespace in version 0.47.
 *
 * @since 0.32.5
 */
class VRKIT_CLASS_API HighlightCoreFinder
{
public:
   typedef boost::function<void (OSG::NodeRefPtr)> callback_t;

   /**
    * Performs a new traversal rooted at the given node and stores the
    * necessary information for later use with material applications. Prior
    * to performing the traversalof \p node, the current state of this object
    * is reset so that previous traversals do not interfere with the new run.
    *
    * @post \c mCores contains pointers to those node cores discovered for
    *       nodes under and including \p node that can handle materials.
    *
    * @param node     The root of the scene graph sub-tree to be traversed.
    * @param callback A callback invoked after a node supporting materials is
    *                 discovered. This is invoked after the node's core is
    *                 added to \c mCores. This parameter is optional and
    *                 defaults to a dummy function that does nothing if not
    *                 specified.
    *
    * @see GeometryHighlightTraverser::addHighlightMaterial()
    * @see GeometryHighlightTraverser::changeHighlightMaterial()
    * @see GeometryHighlightTraverser::removeHighlightMaterial()
    */
   void traverse(OSG::NodePtr node, callback_t callback = defaultCallback);

   /**
    * Clears the internal list of node cores.
    *
    * @post \c mCores is empty.
    */
   void reset();

   /**
    * Returns the vector of node cores found during traversal that support
    * highlighting. If traverse() has not yet been called, then this method
    * returns an empty vector.
    */
   const std::vector<OSG::NodeCoreRefPtr>& getCores() const
   {
      return mCores;
   }

private:
   /**
    * Dummy callback used when traverse() is invoked without a user-supplied
    * callback.
    *
    * @see traverse()
    * @see enter()
    */
   static void defaultCallback(OSG::NodeRefPtr)
   {
      /* Do nothing. */ ;
   }

#if OSG_MAJOR_VERSION < 2
   typedef OSG::NodePtr& traverse_node_type;
#else
   typedef OSG::NodePtrConstArg traverse_node_type;
#endif

   /**
    * Traversal enter method. This is invoked from the OpenSG traverser to
    * build a list of all OSG::MaterialGroup and OSG::Geometry cores.
    *
    * @param node The current node being visited by the traverser.
    */
   OSG::Action::ResultE enter(traverse_node_type node);

   callback_t mCallback;        /**< The callback invoked by enter() */

   /** Cores that support highlighting. */
   std::vector<OSG::NodeCoreRefPtr> mCores;
};

/**
 * Allows sub-trees to have highlight materials applied to them.
 *
 * @note This class was moved into the vrkit::util namespace in version 0.47.
 */
class VRKIT_CLASS_API GeometryHighlightTraverser
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
    * @throw vrkit::Exception
    *           Thrown if either the vertex or the fragment shader files
    *           cannot be read for whatever reason.
    *
    * @see extendShaderSearchPath()
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
    * @throw vrkit::Exception
    *           Thrown if either the vertex or the fragment shader files
    *           cannot be read for whatever reason.
    *
    * @see extendShaderSearchPath()
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
    * Adds the given material to the collection of materials available for
    * highlighting.
    *
    * @return The unique ID for the added material.
    */
   unsigned int registerMaterial(OSG::MaterialRefPtr mat);

   /**
    * Returns the material with the given identifier.
    *
    * @param id The unique ID for the highlight material to be returned.
    */
   OSG::MaterialRefPtr getHighlight(const unsigned int id);

   /**
    * Returns the number of materials currently registered.
    */
   unsigned int getNumMaterials() const;

   /**
    * Determines whether the given material is registered with this traverser.
    *
    * @param mat The material to look up in this traverser's collection of
    *            highlight materials.
    *
    * @return \c true is returned if \p mat is a known highlight material;
    *         otherwise, \c false is returned.
    */
   bool hasHighlight(OSG::MaterialRefPtr mat) const;

   /**
    * Adds the specified highlight material to the sub-tree rooted at the
    * given node.
    *
    * @param node The root of the sub-tree to which the identified material
    *             will be added.
    * @param id   The ID of the highlight material to be added to the
    *             sub-tree rooted at \p node.
    *
    * @throw vrkit::Exception Thrown if \p id is not a valid material ID.
    */
   void addHighlightMaterial(OSG::NodePtr node, const unsigned int id);

   /**
    * Adds the specified highlight material to node cores in the given
    * collection that support materials.
    *
    * @param cores The collection of node cores to be modified.
    * @param id    The ID of the highlight material to be added to the given
    *              node cores.
    *
    * @throw vrkit::Exception Thrown if \p id is not a valid material ID.
    *
    * @see vrkit::HighlightCoreFinder
    *
    * @since 0.32.5
    */
   void addHighlightMaterial(const std::vector<OSG::NodeCoreRefPtr>& cores,
                             const unsigned int id);

   /**
    * Swaps one highlight material (the "old" highlight) for another (the
    * "new" highlight) for the sub-tree rooted at \p node.
    *
    * @param node  The root of the sub-tree whose highlight material will be
    *              replaced.
    * @param oldId The identifier for the highlight material to be removed.
    * @param newId The identifier for the highlight material that will replace
    *              the highlight identified by \p oldId.
    *
    * @throw vrkit::Exception
    *           Thrown if \p oldId or \p newId is not a valid material ID.
    */
   void swapHighlightMaterial(OSG::NodePtr node, const unsigned int oldId,
                              const unsigned int newId);

   /**
    * Swaps one highlight material (the "old" highlight) for another (the
    * "new" highlight) for node cores that support materials in the given
    * collection.
    *
    * @param cores The collection of node cores to be modified.
    * @param oldId The identifier for the highlight material to be removed.
    * @param newId The identifier for the highlight material that will replace
    *              the highlight identified by \p oldId.
    *
    * @throw vrkit::Exception
    *           Thrown if \p oldId or \p newId is not a valid material ID.
    *
    * @since 0.32.5
    */
   void swapHighlightMaterial(const std::vector<OSG::NodeCoreRefPtr>& cores,
                              const unsigned int oldId,
                              const unsigned int newId);

   /**
    * Removes the specified highlight material from the sub-tree rooted at the
    * given node.
    *
    * @param node The root of the sub-tree from which the identified material
    *             will be removed.
    * @param id   The ID of the highlight material to be removed from the
    *             sub-tree rooted at \p node.
    *
    * @throw vrkit::Exception Thrown if \p id is not a valid material ID.
    */
   void removeHighlightMaterial(OSG::NodePtr node, const unsigned int id);

   /**
    * Removes the specified highlight material from the node cores in the
    * given collection that support materials.
    *
    * @param cores The collection of node cores to be modified.
    * @param id    The ID of the highlight material to be removed from the
    *              given node cores.
    *
    * @throw vrkit::Exception Thrown if \p id is not a valid material ID.
    *
    * @see vrkit::HighlightCoreFinder
    *
    * @since 0.32.5
    */
   void removeHighlightMaterial(const std::vector<OSG::NodeCoreRefPtr>& cores,
                                const unsigned int id);

private:
   /**
    * Creates default scribing materials with different diffuse colors.
    */
   void createDefaultMaterials();

   /**
    * Returns the absolute path to the specified shader file.
    *
    * @param filename The basename of the shader file.
    *
    * @returns The absolute path the shader file.
    */
   boost::filesystem::path getCompleteShaderFile(const std::string& filename);

   /**
    * Determines if the specified material ID is valid.
    *
    * @param id The material ID to check.
    *
    * @throw vrkit::Exception is thrown if \p id is an invalid material ID.
    */
   void validateMaterialID(const unsigned int id);

   /** Path used to find named shader. */
   std::vector<boost::filesystem::path> mShaderSearchPath;

   /** Highlight materials that can be assigned. */
   std::vector<OSG::MaterialRefPtr> mMaterials;
};

}

}


#endif /* _VRKIT_UTIL_GEOMETRY_HIGHLIGHT_TRAVERSER_H_ */
