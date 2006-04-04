// Copyright (C) Infiscape Corporation 2005-2006

#include <sstream>
#include <algorithm>
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

#include <IOV/Status.h>

#include <IOV/GeometryHighlightTraverser.h>


namespace fs = boost::filesystem;

namespace
{

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
      OSG::beginEditCP(mpass_mat,
                       OSG::MultiPassMaterial::MaterialsFieldMask);
         mpass_mat->addMaterial(mat);
      OSG::endEditCP(mpass_mat, OSG::MultiPassMaterial::MaterialsFieldMask);

      // Replace the material for the geometry core with the new
      // multi-pass material.
      OSG::beginEditCP(core, CorePtr::StoredObjectType::MaterialFieldMask);
         core->setMaterial(mpass_mat);
      OSG::endEditCP(core, CorePtr::StoredObjectType::MaterialFieldMask);
   }
   // If we already have a multi-pass material, we will use it for
   // mpass_mat.
   else
   {
      mpass_mat = OSG::MultiPassMaterialPtr::dcast(mat);
   }

//   std::cout << "Ading material " << newMat << std::endl;
   // Now, we add the highlight material.
   OSG::beginEditCP(mpass_mat, OSG::MultiPassMaterial::MaterialsFieldMask);
      mpass_mat->addMaterial(newMat);
   OSG::endEditCP(mpass_mat, OSG::MultiPassMaterial::MaterialsFieldMask);
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
      msg_stream << "Node core " << core << " is supposed to have a material";
      throw inf::Exception(msg_stream.str(), IOV_LOCATION);
   }

   OSG::MultiPassMaterialPtr mpass_mat = OSG::MultiPassMaterialPtr::dcast(mat);

   // mat is supposed to be a multi-pass material. If it is not, then
   // something has gone wrong. In this case, either someone is mis-using
   // inf::GeometryHighlightTraverser, or that class has a bug in it.
   if ( OSG::NullFC == mpass_mat )
   {
      std::ostringstream msg_stream;
      msg_stream << "Node core " << core
                 << " is supposed to have an OSG::MultiPassMaterial, but its"
                 << "material is of type " << mat->getType().getCName();
      throw inf::Exception(msg_stream.str(), IOV_LOCATION);
   }

   if ( mpass_mat->hasMaterial(oldMat) )
   {
      OSG::beginEditCP(mpass_mat,
                       OSG::MultiPassMaterial::MaterialsFieldMask);
         mpass_mat->subMaterial(oldMat);
         mpass_mat->addMaterial(newMat);
      OSG::endEditCP(mpass_mat, OSG::MultiPassMaterial::MaterialsFieldMask);
   }
}

template<typename CorePtr>
void removeHighlight(CorePtr core, OSG::MaterialRefPtr oldMaterial)
{
   OSG::MaterialPtr mat = core->getMaterial();

   if ( OSG::NullFC != mat )
   {
      OSG::MultiPassMaterialPtr mpass_mat =
         OSG::MultiPassMaterialPtr::dcast(mat);

      // Ensure that mpass_mat has the given material to be removed and
      // then remove it.
      if ( OSG::NullFC != mpass_mat && mpass_mat->hasMaterial(oldMaterial) )
      {
//         std::cout << "Removing material " << oldMaterial << std::endl;
         OSG::beginEditCP(mpass_mat,
                          OSG::MultiPassMaterial::MaterialsFieldMask);
            mpass_mat->subMaterial(oldMaterial);
         OSG::endEditCP(mpass_mat, OSG::MultiPassMaterial::MaterialsFieldMask);

         // If the number of materials remaining in mpass_mat is 1, then we
         // have reached the point where we need to restore the original
         // material.
         OSG::MFMaterialPtr& materials(mpass_mat->getMaterials());
         if ( materials.getSize() == 1 )
         {
            OSG::MaterialRefPtr orig_mat(mpass_mat->getMaterials(0));
//            std::cout << "Restoring original material " << orig_mat.get()
//                      << std::endl;
            // Restore the material back to whatever it was originally.
            OSG::beginEditCP(core,
                             CorePtr::StoredObjectType::MaterialFieldMask);
               core->setMaterial(orig_mat);
            OSG::endEditCP(core, CorePtr::StoredObjectType::MaterialFieldMask);
         }
      }
   }
}

}

namespace inf
{

GeometryHighlightTraverser::GeometryHighlightTraverser()
{
   createDefaultMaterials();
}

GeometryHighlightTraverser::~GeometryHighlightTraverser()
{
   reset();
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
   throw(inf::Exception)
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

      throw inf::Exception("Failed to find shader programs", IOV_LOCATION);
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
         OSG::CPEditor sce(shlChunk,
                           OSG::ShaderChunk::VertexProgramFieldMask |
                           OSG::ShaderChunk::FragmentProgramFieldMask);

         if ( ! shlChunk->readVertexProgram(vs_file) )
         {
            throw inf::Exception("Failed to read vertex program",
                                 IOV_LOCATION);
         }

         if ( ! shlChunk->readFragmentProgram(fs_file) )
         {
            throw inf::Exception("Failed to read fragment program",
                                 IOV_LOCATION);
         }
      }

      OSG::beginEditCP(shlChunk);
         uniform_map_t::iterator ui;
         for ( ui = uniformParams.begin(); ui != uniformParams.end(); ++ui )
         {
            UniformVisitor visitor(shlChunk, (*ui).first.c_str());
            boost::apply_visitor(visitor, (*ui).second);
         }
      OSG::endEditCP(shlChunk);

      OSG::ChunkMaterialRefPtr chunk_material(OSG::ChunkMaterial::create());
      OSG::beginEditCP(chunk_material, OSG::ChunkMaterial::ChunksFieldMask);
         chunk_material->addChunk(shlChunk);
         std::vector<OSG::StateChunkRefPtr>::const_iterator ci;
         for ( ci = chunks.begin(); ci != chunks.end(); ++ci )
         {
            chunk_material->addChunk(*ci);
         }
      OSG::endEditCP(chunk_material, OSG::ChunkMaterial::ChunksFieldMask);

      OSG::MaterialRefPtr material(chunk_material.get());
      id = mMaterials.size();
      mMaterials.push_back(material);
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

   OSG::beginEditCP(mat, OSG::SimpleMaterial::LitFieldMask |
                         OSG::SimpleMaterial::DiffuseFieldMask);
      mat->setLit(isLit);
      mat->setDiffuse(diffuseColor);
   OSG::endEditCP(mat, OSG::SimpleMaterial::LitFieldMask |
                       OSG::SimpleMaterial::DiffuseFieldMask);

   OSG::PolygonChunkPtr scribe_chunk = OSG::PolygonChunk::create();
   OSG::beginEditCP(scribe_chunk);
      scribe_chunk->setFrontMode(frontMode);
      scribe_chunk->setOffsetLine(offsetLine);
      scribe_chunk->setOffsetFill(offsetFill);
      scribe_chunk->setOffsetPoint(offsetPoint);
      scribe_chunk->setOffsetFactor(offsetFactor);
      scribe_chunk->setOffsetBias(offsetBias);
   OSG::endEditCP(scribe_chunk);

   OSG::beginEditCP(mat, OSG::SimpleMaterial::ChunksFieldMask);
      mat->addChunk(scribe_chunk);
   OSG::endEditCP(mat, OSG::SimpleMaterial::ChunksFieldMask);

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
   throw(inf::Exception)
{
   validateMaterialID(id);

   traverse(node);

   OSG::MaterialRefPtr material(mMaterials[id]);

   std::vector<OSG::GeometryRefPtr>::iterator gc;
   for ( gc = mGeomCores.begin(); gc != mGeomCores.end(); ++gc )
   {
      addHighlight((*gc).get(), material);
   }

   std::vector<OSG::MaterialGroupRefPtr>::iterator mgc;
   for ( mgc = mMatGroupCores.begin(); mgc != mMatGroupCores.end(); ++mgc )
   {
      addHighlight((*mgc).get(), material);
   }
}

void GeometryHighlightTraverser::swapHighlightMaterial(OSG::NodePtr node,
                                                       const unsigned int oldID,
                                                       const unsigned int newID)
   throw(inf::Exception)
{
   validateMaterialID(oldID);
   validateMaterialID(newID);

   OSG::MaterialRefPtr old_mat(mMaterials[oldID]);
   OSG::MaterialRefPtr new_mat(mMaterials[newID]);

   if ( old_mat == new_mat )
   {
      IOV_STATUS << "WARNING: Trying to replace material with itself!"
                 << std::endl;
      return;
   }

   // Traverse node to populate mGeomCores and mMatGroupCores.
   traverse(node);

   std::vector<OSG::GeometryRefPtr>::iterator gc;
   for ( gc = mGeomCores.begin(); gc != mGeomCores.end(); ++gc )
   {
      try
      {
         swapHighlight(*gc, old_mat, new_mat);
      }
      catch (inf::Exception& ex)
      {
         IOV_STATUS
            << "[inf::GeometryHighlightTraverser::swapHighlightMaterial()] "
            << "WARNING:\n" << ex.what() << std::endl;
      }
   }

   std::vector<OSG::MaterialGroupRefPtr>::iterator mgc;
   for ( mgc = mMatGroupCores.begin(); mgc != mMatGroupCores.end(); ++mgc )
   {
      try
      {
         swapHighlight(*mgc, old_mat, new_mat);
      }
      catch (inf::Exception& ex)
      {
         IOV_STATUS
            << "[inf::GeometryHighlightTraverser::swapHighlightMaterial()] "
            << "WARNING:\n" << ex.what() << std::endl;
      }
   }
}

void GeometryHighlightTraverser::removeHighlightMaterial(OSG::NodePtr node,
                                                         const unsigned int id)
   throw(inf::Exception)
{
   validateMaterialID(id);

   traverse(node);

   OSG::MaterialRefPtr old_mat(mMaterials[id]);

   std::vector<OSG::GeometryRefPtr>::iterator gc;
   for ( gc = mGeomCores.begin(); gc != mGeomCores.end(); ++gc )
   {
      removeHighlight(*gc, old_mat);
   }

   std::vector<OSG::MaterialGroupRefPtr>::iterator mgc;
   for ( mgc = mMatGroupCores.begin(); mgc != mMatGroupCores.end(); ++mgc )
   {
      removeHighlight(*mgc, old_mat);
   }
}

void GeometryHighlightTraverser::createDefaultMaterials()
{
   mMaterials.resize(LAST_HIGHLIGHT);

   // Set up the highlight materials.
   OSG::SimpleMaterialPtr highlight0_mat =
      OSG::SimpleMaterial::create();

   OSG::beginEditCP(highlight0_mat);
      highlight0_mat->setLit(false);
      highlight0_mat->setDiffuse(OSG::Color3f(1.0f, 1.0f, 0.0f));
   OSG::endEditCP(highlight0_mat);

   OSG::SimpleMaterialPtr highlight1_mat =
      OSG::SimpleMaterial::create();

   OSG::beginEditCP(highlight1_mat);
      highlight1_mat->setLit(false);
      highlight1_mat->setDiffuse(OSG::Color3f(1.0f, 0.0f, 1.0f));
   OSG::endEditCP(highlight1_mat);

   OSG::PolygonChunkPtr scribe_chunk = OSG::PolygonChunk::create();
   OSG::beginEditCP(scribe_chunk);
      scribe_chunk->setFrontMode(GL_LINE);
      scribe_chunk->setOffsetLine(true);
      scribe_chunk->setOffsetFill(false);
      scribe_chunk->setOffsetPoint(false);
      scribe_chunk->setOffsetFactor(0.05f);
      scribe_chunk->setOffsetBias(1.0f);
   OSG::endEditCP(scribe_chunk);

   OSG::beginEditCP(highlight0_mat);
      highlight0_mat->addChunk(scribe_chunk);
   OSG::endEditCP(highlight0_mat);

   OSG::beginEditCP(highlight1_mat);
      highlight1_mat->addChunk(scribe_chunk);
   OSG::endEditCP(highlight1_mat);

   mMaterials[HIGHLIGHT0] = highlight0_mat;
   mMaterials[HIGHLIGHT1] = highlight1_mat;
}

void GeometryHighlightTraverser::reset()
{
   mGeomCores.clear();
   mMatGroupCores.clear();
}

void GeometryHighlightTraverser::traverse(OSG::NodePtr node)
{
   reset();
   OSG::traverse(node,
                 OSG::osgTypedMethodFunctor1ObjPtrCPtrRef<
                    OSG::Action::ResultE, GeometryHighlightTraverser,
                    OSG::NodePtr
                 >(this, &GeometryHighlightTraverser::enter));
}

OSG::Action::ResultE GeometryHighlightTraverser::enter(OSG::NodePtr& node)
{
   OSG::NodeCorePtr core = node->getCore();
   OSG::FieldContainerType& core_fct = core->getType();

   if ( core_fct.isDerivedFrom(OSG::Geometry::getClassType()) )
   {
      OSG::GeometryPtr geom = OSG::GeometryPtr::dcast(core);
      mGeomCores.push_back(OSG::GeometryRefPtr(geom));
   }
   else if ( core_fct.isDerivedFrom(OSG::MaterialGroup::getClassType()) )
   {
      OSG::MaterialGroupPtr mat_grp = OSG::MaterialGroupPtr::dcast(core);
      mMatGroupCores.push_back(OSG::MaterialGroupRefPtr(mat_grp));
   }

   return OSG::Action::Continue;
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
   throw(inf::Exception)
{
   if ( id >= mMaterials.size() )
   {
      std::ostringstream msg_stream;
      msg_stream << "Material ID " << id << " not in range [0,"
                 << mMaterials.size() << "]";
      throw inf::Exception(msg_stream.str(), IOV_LOCATION);
   }
}

}
