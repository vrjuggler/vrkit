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

#include <sstream>
#include <algorithm>
#include <boost/bind.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/exception.hpp>

#include <OpenSG/OSGNodeCore.h>
#include <OpenSG/OSGGeometry.h>
#include <OpenSG/OSGMultiPassMaterial.h>
#include <OpenSG/OSGSimpleGeometry.h>
#include <OpenSG/OSGSimpleMaterial.h>
#include <OpenSG/OSGSHLChunk.h>
#include <OpenSG/OSGPolygonChunk.h>

#include <vrkit/Status.h>
#include <vrkit/Exception.h>
#include <vrkit/util/GeometryHighlightTraverser.h>


namespace fs = boost::filesystem;

namespace
{

/**
 * Static visitor that is used to set uniform parameters on SHL chunks.
 */
class UniformVisitor : public boost::static_visitor<>
{
public:
   UniformVisitor(OSG::SHLChunkRefPtr shlChunk, const char* name)
      : mSHLChunk(shlChunk)
      , mName(name)
   {
      /* Do nothing. */ ;
   }

   ~UniformVisitor()
   {
      /* Do nothing. */ ;
   }

   void operator()(bool& b) const
   {
      mSHLChunk->setUniformParameter(mName, b);
   }

   void operator()(OSG::Int32& i) const
   {
      mSHLChunk->setUniformParameter(mName, i);
   }

   void operator()(OSG::Real32& r) const
   {
      mSHLChunk->setUniformParameter(mName, r);
   }

   void operator()(OSG::Vec2f& v) const
   {
      mSHLChunk->setUniformParameter(mName, v);
   }

   void operator()(OSG::Vec3f& v) const
   {
      mSHLChunk->setUniformParameter(mName, v);
   }

   void operator()(OSG::Vec4f& v) const
   {
      mSHLChunk->setUniformParameter(mName, v);
   }

private:
   OSG::SHLChunkRefPtr mSHLChunk;
   const char*         mName;
};

template<typename CorePtr>
void addHighlight(CorePtr core, OSG::MaterialRefPtr newMat)
{
   OSG::MultiPassMaterialPtr mpass_mat;

   // Save the core's current material so that we can restore it later
   // in removeHighlightMaterial().
   OSG::MaterialPtr mat = core->getMaterial();

   // If the geometry node has no material at all or its material is not a
   // multi-pass material, then we need to create a new mulit-pass material
   // and make the geometry's current material the first material of
   // mpass_mat.
   if ( OSG::NullFC == mat ||
        ! mat->getType().isDerivedFrom(OSG::MultiPassMaterial::getClassType()) )
   {
      // If the geometry node has no material at all, we have to inject a
      // dummy material to ensure that the geometry is rendered before the
      // highlighting is rendered.
      if ( OSG::NullFC == mat )
      {
         mat = OSG::getDefaultMaterial();
      }

      mpass_mat = OSG::MultiPassMaterial::create();
#if OSG_MAJOR_VERSION < 2
      OSG::CPEditor mme(mpass_mat, OSG::MultiPassMaterial::MaterialsFieldMask);
#endif
      mpass_mat->addMaterial(mat);

      // Replace the material for the geometry core with the new
      // multi-pass material.
#if OSG_MAJOR_VERSION < 2
      OSG::CPEditor ce(core, CorePtr::StoredObjectType::MaterialFieldMask);
#endif
      core->setMaterial(mpass_mat);
   }
   // If we already have a multi-pass material, we will use it for
   // mpass_mat.
   else
   {
      mpass_mat =
#if OSG_MAJOR_VERSION < 2
         OSG::MultiPassMaterialPtr::dcast(mat);
#else
         OSG::cast_dynamic<OSG::MultiPassMaterialPtr>(mat);
#endif
   }

//   std::cout << "Ading material " << newMat << std::endl;
   // Now, we add the highlight material.
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor mme(mpass_mat, OSG::MultiPassMaterial::MaterialsFieldMask);
#endif
   mpass_mat->addMaterial(newMat);
}

template<typename CorePtr>
void swapHighlight(CorePtr core, OSG::MaterialRefPtr oldMat,
                   OSG::MaterialRefPtr newMat)
{
   OSG::MaterialPtr mat = core->getMaterial();

   // Verify that core actually has a material set. If it does not, then
   // there is nothing we can do.
   if ( OSG::NullFC == mat )
   {
      std::ostringstream msg_stream;
      msg_stream << "Node core " << (OSG::FieldContainerPtr) core
                 << " is supposed to have a material";
      throw vrkit::Exception(msg_stream.str(), VRKIT_LOCATION);
   }

   OSG::MultiPassMaterialPtr mpass_mat =
#if OSG_MAJOR_VERSION < 2
      OSG::MultiPassMaterialPtr::dcast(mat);
#else
      OSG::cast_dynamic<OSG::MultiPassMaterialPtr>(mat);
#endif

   // mat is supposed to be a multi-pass material. If it is not, then
   // something has gone wrong. In this case, either someone is mis-using
   // vrkit::GeometryHighlightTraverser, or that class has a bug in it.
   if ( OSG::NullFC == mpass_mat )
   {
      std::ostringstream msg_stream;
      msg_stream << "Node core " << (OSG::FieldContainerPtr) core
                 << " is supposed to have an OSG::MultiPassMaterial, but its"
                 << "material is of type " << mat->getType().getCName();
      throw vrkit::Exception(msg_stream.str(), VRKIT_LOCATION);
   }

   if ( mpass_mat->hasMaterial(oldMat) )
   {
#if OSG_MAJOR_VERSION < 2
      OSG::CPEditor mme(mpass_mat, OSG::MultiPassMaterial::MaterialsFieldMask);
#endif
      mpass_mat->subMaterial(oldMat);
      mpass_mat->addMaterial(newMat);
   }
}

template<typename CorePtr>
void removeHighlight(CorePtr core, OSG::MaterialRefPtr oldMaterial)
{
   OSG::MaterialPtr mat = core->getMaterial();

   if ( OSG::NullFC != mat )
   {
      OSG::MultiPassMaterialPtr mpass_mat =
#if OSG_MAJOR_VERSION < 2
         OSG::MultiPassMaterialPtr::dcast(mat);
#else
         OSG::cast_dynamic<OSG::MultiPassMaterialPtr>(mat);
#endif

      // Ensure that mpass_mat has the given material to be removed and
      // then remove it.
      if ( OSG::NullFC != mpass_mat && mpass_mat->hasMaterial(oldMaterial) )
      {
//         std::cout << "Removing material " << oldMaterial << std::endl;
#if OSG_MAJOR_VERSION < 2
         OSG::CPEditor mme(mpass_mat,
                           OSG::MultiPassMaterial::MaterialsFieldMask);
#endif
         mpass_mat->subMaterial(oldMaterial);

         // If the number of materials remaining in mpass_mat is 1, then we
         // have reached the point where we need to restore the original
         // material.
         const OSG::MFMaterialPtr& materials(mpass_mat->getMaterials());
         const OSG::UInt32 materials_size =
#if OSG_MAJOR_VERSION < 2
            materials.getSize();
#else
            materials.size();
#endif

         if ( materials_size == 1 )
         {
            OSG::MaterialRefPtr orig_mat(mpass_mat->getMaterials(0));
//            std::cout << "Restoring original material " << orig_mat.get()
//                      << std::endl;
            // Restore the material back to whatever it was originally.
#if OSG_MAJOR_VERSION < 2
            OSG::CPEditor ce(core,
                             CorePtr::StoredObjectType::MaterialFieldMask);
#endif
            core->setMaterial(orig_mat);
         }
      }
   }
}

}

namespace vrkit
{

namespace util
{

void HighlightCoreFinder::traverse(OSG::NodePtr node, callback_t callback)
{
   reset();
   mCallback = callback;
   OSG::traverse(node,
#if OSG_MAJOR_VERSION < 2
                 OSG::osgTypedMethodFunctor1ObjPtrCPtrRef<
                    OSG::Action::ResultE, HighlightCoreFinder, OSG::NodePtr
                 >(this, &HighlightCoreFinder::enter)
#else
                 boost::bind(&HighlightCoreFinder::enter, this, _1)
#endif
   );
}

void HighlightCoreFinder::reset()
{
   mCores.clear();
}

OSG::Action::ResultE HighlightCoreFinder::enter(traverse_node_type node)
{
   OSG::NodeCorePtr core = node->getCore();
   OSG::FieldContainerType& core_fct = core->getType();

   if ( core_fct.isDerivedFrom(OSG::Geometry::getClassType()) ||
        core_fct.isDerivedFrom(OSG::MaterialGroup::getClassType()) )
   {
      mCores.push_back(OSG::NodeCoreRefPtr(core));
      mCallback(OSG::NodeRefPtr(node));
   }

   return OSG::Action::Continue;
}

GeometryHighlightTraverser::GeometryHighlightTraverser()
{
   createDefaultMaterials();
}

GeometryHighlightTraverser::~GeometryHighlightTraverser()
{
   /* Do nothing. */ ;
}

void GeometryHighlightTraverser::
extendShaderSearchPath(const std::vector<fs::path>& path)
{
   mShaderSearchPath.insert(mShaderSearchPath.end(), path.begin(), path.end());
}

unsigned int GeometryHighlightTraverser::
createSHLMaterial(const std::string& vertexShaderFile,
                  const std::string& fragmentShaderFile,
                  const std::vector<OSG::StateChunkRefPtr>& chunks,
                  GeometryHighlightTraverser::uniform_map_t& uniformParams,
                  OSG::SHLChunkRefPtr shlChunk)
{
   unsigned int id(0);
   fs::path vs_file_path(getCompleteShaderFile(vertexShaderFile));
   fs::path fs_file_path(getCompleteShaderFile(fragmentShaderFile));

   fs::ifstream vs_file(vs_file_path);
   fs::ifstream fs_file(fs_file_path);

   if ( ! vs_file || ! fs_file )
   {
      if ( ! vs_file )
      {
         std::cerr << "WARNING: Could not open '"
                   << vs_file_path.string() << "'" << std::endl;
      }

      if ( ! fs_file )
      {
         std::cerr << "WARNING: Could not open '"
                   << fs_file_path.string() << "'" << std::endl;
      }

      throw Exception("Failed to find shader programs", VRKIT_LOCATION);
   }
   else
   {
      if ( OSG::NullFC == shlChunk.get() )
      {
         shlChunk = OSG::SHLChunk::create();
      }

      {
         // CPEditor for shlChunk. This is important because an exception may
         // be thrown while editing shlChunk.
#if OSG_MAJOR_VERSION < 2
         OSG::CPEditor sce(shlChunk,
                           OSG::ShaderChunk::VertexProgramFieldMask |
                              OSG::ShaderChunk::FragmentProgramFieldMask);
#endif

         if ( ! shlChunk->readVertexProgram(vs_file) )
         {
            throw Exception("Failed to read vertex program", VRKIT_LOCATION);
         }

         if ( ! shlChunk->readFragmentProgram(fs_file) )
         {
            throw Exception("Failed to read fragment program", VRKIT_LOCATION);
         }
      }

      {
#if OSG_MAJOR_VERSION < 2
         OSG::CPEditor sce(shlChunk);
#endif
         typedef uniform_map_t::iterator iter_type;
         for ( iter_type ui = uniformParams.begin();
               ui != uniformParams.end();
               ++ui )
         {
            UniformVisitor visitor(shlChunk, (*ui).first.c_str());
            boost::apply_visitor(visitor, (*ui).second);
         }
      }

      OSG::ChunkMaterialRefPtr chunk_material(OSG::ChunkMaterial::create());
      {
#if OSG_MAJOR_VERSION < 2
         OSG::CPEditor cme(chunk_material,
                           OSG::ChunkMaterial::ChunksFieldMask);
#endif
         chunk_material->addChunk(shlChunk);
         typedef std::vector<OSG::StateChunkRefPtr>::const_iterator iter_type;
         for ( iter_type ci = chunks.begin(); ci != chunks.end(); ++ci )
         {
            chunk_material->addChunk(*ci);
         }
      }

      OSG::MaterialRefPtr material(chunk_material.get());
      id = registerMaterial(material);
   }

   return id;
}

unsigned int GeometryHighlightTraverser::
createScribeMaterial(const bool isLit, const unsigned int frontMode,
                     const bool offsetLine, const bool offsetFill,
                     const bool offsetPoint, const float offsetFactor,
                     const float offsetBias, const OSG::Color3f& diffuseColor)
{
   // Set up the highlight materials.
   OSG::SimpleMaterialPtr mat = OSG::SimpleMaterial::create();

#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor me(mat,
                    OSG::SimpleMaterial::LitFieldMask |
                       OSG::SimpleMaterial::DiffuseFieldMask |
                       OSG::SimpleMaterial::ChunksFieldMask);
#endif
   mat->setLit(isLit);
   mat->setDiffuse(diffuseColor);

   OSG::PolygonChunkPtr scribe_chunk = OSG::PolygonChunk::create();
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor sce(scribe_chunk);
#endif
   scribe_chunk->setFrontMode(frontMode);
   scribe_chunk->setOffsetLine(offsetLine);
   scribe_chunk->setOffsetFill(offsetFill);
   scribe_chunk->setOffsetPoint(offsetPoint);
   scribe_chunk->setOffsetFactor(offsetFactor);
   scribe_chunk->setOffsetBias(offsetBias);

   mat->addChunk(scribe_chunk);

   return registerMaterial(OSG::MaterialRefPtr(mat));
}

unsigned int GeometryHighlightTraverser::
registerMaterial(OSG::MaterialRefPtr mat)
{
   unsigned int id(mMaterials.size());
   mMaterials.push_back(mat);
   return id;
}

OSG::MaterialRefPtr GeometryHighlightTraverser::getHighlight(const unsigned int id)
{
   return mMaterials[id];
}

unsigned int GeometryHighlightTraverser::getNumMaterials() const
{
   return mMaterials.size();
}

bool GeometryHighlightTraverser::hasHighlight(OSG::MaterialRefPtr mat) const
{
   bool result(false);
   std::vector<OSG::MaterialRefPtr>::const_iterator i;
   for ( i = mMaterials.begin(); i != mMaterials.end(); ++i )
   {
      if ( (*i).get() == mat.get() )
      {
         result = true;
         break;
      }
   }
   return result;

   // XXX: This does not compile for some reason, even with an
   // overload of operator== for OSG::MaterialRefPtr.
//   return std::find(mMaterials.begin(), mMaterials.end(), mat) != mMaterials.end();
}

void GeometryHighlightTraverser::addHighlightMaterial(OSG::NodePtr node,
                                                      const unsigned int id)
{
   HighlightCoreFinder finder;
   finder.traverse(node);
   addHighlightMaterial(finder.getCores(), id);
}

void GeometryHighlightTraverser::
addHighlightMaterial(const std::vector<OSG::NodeCoreRefPtr>& cores,
                     const unsigned int id)
{
   validateMaterialID(id);

   OSG::MaterialRefPtr material(mMaterials[id]);

   typedef std::vector<OSG::NodeCoreRefPtr>::const_iterator iter_type;
   for ( iter_type c = cores.begin(); c != cores.end(); ++c )
   {
      OSG::FieldContainerType& core_fct = (*c)->getType();

      if ( core_fct.isDerivedFrom(OSG::Geometry::getClassType()) )
      {
         addHighlight(
#if OSG_MAJOR_VERSION < 2
            OSG::GeometryPtr::dcast((*c).get()),
#else
            OSG::cast_dynamic<OSG::GeometryPtr>((*c).get()),
#endif
            material
         );
      }
      else if ( core_fct.isDerivedFrom(OSG::MaterialGroup::getClassType()) )
      {
         addHighlight(
#if OSG_MAJOR_VERSION < 2
            OSG::MaterialGroupPtr::dcast((*c).get()),
#else
            OSG::cast_dynamic<OSG::MaterialGroupPtr>((*c).get()),
#endif
            material
         );
      }
   }
}

void GeometryHighlightTraverser::swapHighlightMaterial(OSG::NodePtr node,
                                                       const unsigned int oldID,
                                                       const unsigned int newID)
{
   HighlightCoreFinder finder;
   finder.traverse(node);
   swapHighlightMaterial(finder.getCores(), oldID, newID);
}

void GeometryHighlightTraverser::
swapHighlightMaterial(const std::vector<OSG::NodeCoreRefPtr>& cores,
                      const unsigned int oldID, const unsigned int newID)
{
   validateMaterialID(oldID);
   validateMaterialID(newID);

   OSG::MaterialRefPtr old_mat(mMaterials[oldID]);
   OSG::MaterialRefPtr new_mat(mMaterials[newID]);

   if ( old_mat == new_mat )
   {
      VRKIT_STATUS << "WARNING: Trying to replace material with itself!"
                   << std::endl;
      return;
   }

   typedef std::vector<OSG::NodeCoreRefPtr>::const_iterator iter_type;
   for ( iter_type c = cores.begin(); c != cores.end(); ++c )
   {
      OSG::FieldContainerType& core_fct = (*c)->getType();

      try
      {
         if ( core_fct.isDerivedFrom(OSG::Geometry::getClassType()) )
         {
            swapHighlight(
#if OSG_MAJOR_VERSION < 2
               OSG::GeometryPtr::dcast((*c).get()),
#else
               OSG::cast_dynamic<OSG::GeometryPtr>((*c).get()),
#endif
               old_mat, new_mat
            );
         }
         else if ( core_fct.isDerivedFrom(OSG::MaterialGroup::getClassType()) )
         {
            swapHighlight(
#if OSG_MAJOR_VERSION < 2
               OSG::MaterialGroupPtr::dcast((*c).get()),
#else
               OSG::cast_dynamic<OSG::MaterialGroupPtr>((*c).get()),
#endif
               old_mat, new_mat
            );
         }
      }
      catch (Exception& ex)
      {
         VRKIT_STATUS
            << "[vrkit::GeometryHighlightTraverser::swapHighlightMaterial()] "
            << "WARNING:\n" << ex.what() << std::endl;
      }
   }
}

void GeometryHighlightTraverser::removeHighlightMaterial(OSG::NodePtr node,
                                                         const unsigned int id)
{
   HighlightCoreFinder finder;
   finder.traverse(node);
   removeHighlightMaterial(finder.getCores(), id);
}

void GeometryHighlightTraverser::
removeHighlightMaterial(const std::vector<OSG::NodeCoreRefPtr>& cores,
                        const unsigned int id)
{
   validateMaterialID(id);

   OSG::MaterialRefPtr old_mat(mMaterials[id]);

   typedef std::vector<OSG::NodeCoreRefPtr>::const_iterator iter_type;
   for ( iter_type c = cores.begin(); c != cores.end(); ++c )
   {
      OSG::FieldContainerType& core_fct = (*c)->getType();

      if ( core_fct.isDerivedFrom(OSG::Geometry::getClassType()) )
      {
         removeHighlight(
#if OSG_MAJOR_VERSION < 2
            OSG::GeometryPtr::dcast((*c).get()),
#else
            OSG::cast_dynamic<OSG::GeometryPtr>((*c).get()),
#endif
            old_mat
         );
      }
      else if ( core_fct.isDerivedFrom(OSG::MaterialGroup::getClassType()) )
      {
         removeHighlight(
#if OSG_MAJOR_VERSION < 2
            OSG::MaterialGroupPtr::dcast((*c).get()),
#else
            OSG::cast_dynamic<OSG::MaterialGroupPtr>((*c).get()),
#endif
            old_mat
         );
      }
   }
}

void GeometryHighlightTraverser::createDefaultMaterials()
{
   mMaterials.resize(LAST_HIGHLIGHT);

   // Set up the highlight materials.
   OSG::SimpleMaterialPtr highlight0_mat =
      OSG::SimpleMaterial::create();

#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor h0me(highlight0_mat,
                      OSG::SimpleMaterial::LitFieldMask |
                         OSG::SimpleMaterial::DiffuseFieldMask |
                         OSG::SimpleMaterial::ChunksFieldMask);
#endif
   highlight0_mat->setLit(false);
   highlight0_mat->setDiffuse(OSG::Color3f(1.0f, 1.0f, 0.0f));

   OSG::SimpleMaterialPtr highlight1_mat =
      OSG::SimpleMaterial::create();

#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor h1me(highlight1_mat,
                      OSG::SimpleMaterial::LitFieldMask |
                         OSG::SimpleMaterial::DiffuseFieldMask |
                         OSG::SimpleMaterial::ChunksFieldMask);
#endif
   highlight1_mat->setLit(false);
   highlight1_mat->setDiffuse(OSG::Color3f(1.0f, 0.0f, 1.0f));

   OSG::PolygonChunkPtr scribe_chunk = OSG::PolygonChunk::create();
#if OSG_MAJOR_VERSION < 2
   OSG::CPEditor sce(scribe_chunk);
#endif
   scribe_chunk->setFrontMode(GL_LINE);
   scribe_chunk->setOffsetLine(true);
   scribe_chunk->setOffsetFill(false);
   scribe_chunk->setOffsetPoint(false);
   scribe_chunk->setOffsetFactor(0.05f);
   scribe_chunk->setOffsetBias(1.0f);

   highlight0_mat->addChunk(scribe_chunk);
   highlight1_mat->addChunk(scribe_chunk);

   mMaterials[HIGHLIGHT0] = highlight0_mat;
   mMaterials[HIGHLIGHT1] = highlight1_mat;
}

fs::path GeometryHighlightTraverser::
getCompleteShaderFile(const std::string& filename)
{
   fs::path file_path(filename, fs::native);

   if ( ! file_path.is_complete() )
   {
      for ( std::vector<fs::path>::iterator i = mShaderSearchPath.begin();
            i != mShaderSearchPath.end();
            ++i )
      {
         try
         {
            fs::path cur_path(*i / filename);
            if ( fs::exists(cur_path) )
            {
               file_path = cur_path;
               break;
            }
         }
         catch (fs::filesystem_error& ex)
         {
            std::cerr << ex.what() << std::endl;
         }
      }
   }

   return file_path;
}

void GeometryHighlightTraverser::validateMaterialID(const unsigned int id)
{
   if ( id >= mMaterials.size() )
   {
      std::ostringstream msg_stream;
      msg_stream << "Material ID " << id << " not in range [0,"
                 << mMaterials.size() << "]";
      throw Exception(msg_stream.str(), VRKIT_LOCATION);
   }
}

}

}
