#include <IOV/UiBuilder.h>
#include <OpenSG/OSGSimpleMaterial.h>
#include <OpenSG/OSGDepthChunk.h>

namespace inf
{

OSG::GeometryPtr UiBuilder::createGeomGeo()
{
   OSG::GeometryPtr geo_core = OSG::Geometry::create();

   OSG::GeoPTypesPtr type = OSG::GeoPTypesUI8::create();
   OSG::GeoPLengthsPtr lens = OSG::GeoPLengthsUI32::create();
   OSG::GeoPositions3fPtr pnts = OSG::GeoPositions3f::create();
   OSG::GeoColors4fPtr colors = OSG::GeoColors4f::create();
   OSG::GeoNormals3fPtr norms = OSG::GeoNormals3f::create();

   OSG::ChunkMaterialPtr mat = OSG::ChunkMaterial::create();
   OSG::MaterialChunkPtr mat_chunk = OSG::MaterialChunk::create();
   OSG::BlendChunkPtr blend_chunk = OSG::BlendChunk::create();
   OSG::DepthChunkPtr depth_chunk = OSG::DepthChunk::create();
   OSG::CPEditor mat_ed(mat), mce(mat_chunk), bce(blend_chunk), dce(depth_chunk);

   mat_chunk->setLit(true);
   mat_chunk->setColorMaterial(GL_AMBIENT_AND_DIFFUSE);
   mat_chunk->setSpecular(OSG::Color4f(0,0,0,1));

   blend_chunk->setSrcFactor(GL_SRC_ALPHA);
   blend_chunk->setDestFactor(GL_ONE_MINUS_SRC_ALPHA);

   depth_chunk->setEnable(true);

   OSG::beginEditCP(mat);
      mat->addChunk(mat_chunk);
      mat->addChunk(blend_chunk);
      mat->addChunk(depth_chunk);
   OSG::endEditCP(mat);

   // XXX: Handle transparency

   OSG::beginEditCP(geo_core);
   {
       geo_core->setTypes(type);
       geo_core->setLengths(lens);
       geo_core->setPositions(pnts);
       geo_core->setColors(colors);
       geo_core->setNormals(norms);

       // assign a material to the geometry to make it visible. The details
       // of materials are defined later.
       geo_core->setMaterial(mat);
   }
   OSG::endEditCP(geo_core);

   return geo_core;
}

void UiBuilder::buildRectangle(OSG::GeometryPtr geom, OSG::Color3f color, OSG::Pnt2f minPt, OSG::Pnt2f maxPt, float alpha, bool filled)
{
   OSG::GeoPTypesPtr types = OSG::GeoPTypesUI8Ptr::dcast(geom->getTypes());
   OSG::GeoPLengthsPtr lens = OSG::GeoPLengthsUI32Ptr::dcast(geom->getLengths());
   OSG::GeoPositions3fPtr verts = OSG::GeoPositions3fPtr::dcast(geom->getPositions());
   OSG::GeoColors4fPtr colors = OSG::GeoColors4fPtr::dcast(geom->getColors());
   OSG::GeoNormals3fPtr norms = OSG::GeoNormals3fPtr::dcast(geom->getNormals());

   assert(OSG::NullFC != types);
   assert(OSG::NullFC != lens);
   assert(OSG::NullFC != verts);
   assert(OSG::NullFC != colors);
   assert(OSG::NullFC != norms);

   OSG::MFColor4f* mfc = colors->getFieldPtr();
   assert(NULL != mfc);

   OSG::CPEditor types_ed(types);
   OSG::CPEditor lens_ed(lens);
   OSG::CPEditor verts_ed(verts);
   OSG::CPEditor colors_ed(colors);
   OSG::CPEditor norms_ed(norms);

   OSG::Pnt3f ll(minPt.x(), minPt.y(), 0.0f), lr(maxPt.x(), minPt.y(), 0.0f),
              ul(minPt.x(), maxPt.y(), 0.0f), ur(maxPt.x(), maxPt.y(), 0.0f);
   OSG::Color4f used_color(color.red(), color.green(), color.blue(), alpha);

   if(filled)
   {
      const unsigned num_verts(2*3);

      types->addValue(GL_TRIANGLES);
      lens->addValue(num_verts);
      verts->addValue(lr); verts->addValue(ur); verts->addValue(ul);
      verts->addValue(ll); verts->addValue(lr); verts->addValue(ul);
      for(unsigned i=0;i<num_verts;++i)
      {
         mfc->push_back(used_color);
         norms->addValue(OSG::Vec3f(0,0,-1));
      }
   }
   else  // Line strip
   {
      const unsigned num_verts(4);

      types->addValue(GL_LINE_STRIP);
      lens->addValue(num_verts);
      verts->addValue(lr); verts->addValue(ur);
      verts->addValue(ul); verts->addValue(ll);
      for(unsigned i=0;i<num_verts;++i)
      {
         mfc->push_back(used_color);
         norms->addValue(OSG::Vec3f(0,0,-1));
      }
   }
}

void UiBuilder::buildBox(OSG::GeometryPtr geom, OSG::Color3f color, OSG::Pnt2f minPt, OSG::Pnt2f maxPt, float depth, float alpha)
{
   OSG::GeoPTypesPtr types = OSG::GeoPTypesUI8Ptr::dcast(geom->getTypes());
   OSG::GeoPLengthsPtr lens = OSG::GeoPLengthsUI32Ptr::dcast(geom->getLengths());
   OSG::GeoPositions3fPtr verts = OSG::GeoPositions3fPtr::dcast(geom->getPositions());
   OSG::GeoColors4fPtr colors = OSG::GeoColors4fPtr::dcast(geom->getColors());
   OSG::GeoNormals3fPtr norms = OSG::GeoNormals3fPtr::dcast(geom->getNormals());

   assert(OSG::NullFC != types);
   assert(OSG::NullFC != lens);
   assert(OSG::NullFC != verts);
   assert(OSG::NullFC != colors);
   assert(OSG::NullFC != norms);

   OSG::MFColor4f* mfc = colors->getFieldPtr();
   assert(NULL != mfc);

   OSG::Color4f used_color(color.red(), color.green(), color.blue(), alpha);

   OSG::CPEditor types_ed(types);
   OSG::CPEditor lens_ed(lens);
   OSG::CPEditor verts_ed(verts);
   OSG::CPEditor colors_ed(colors);
   OSG::CPEditor norms_ed(norms);

   float z_offset(depth/2.0f);
   OSG::Pnt3f corners[6];
   corners[0].setValues(minPt.x(), minPt.y(), z_offset); // llf
   corners[1].setValues(maxPt.x(), minPt.y(), z_offset); // lrf
   corners[2].setValues(minPt.x(), maxPt.y(), z_offset); // ulf
   corners[3].setValues(maxPt.x(), maxPt.y(), z_offset); // urf
   corners[4].setValues(minPt.x(), minPt.y(), -z_offset); // llb
   corners[5].setValues(maxPt.x(), minPt.y(), -z_offset); // lrb
   corners[6].setValues(minPt.x(), maxPt.y(), -z_offset); // ulb
   corners[7].setValues(maxPt.x(), maxPt.y(), -z_offset); // urb

   #define ADD_TRI(a,b,c) {                   \
      OSG::Vec3f e1 = corners[b]-corners[a];  \
      OSG::Vec3f e2 = corners[c]-corners[a];  \
      OSG::Vec3f norm = e1.cross(e2);         \
      norm.normalize();                         \
      verts->addValue(corners[a]);            \
      verts->addValue(corners[b]);            \
      verts->addValue(corners[c]);            \
      mfc->push_back(used_color); mfc->push_back(used_color); mfc->push_back(used_color);  \
      norms->addValue(norm); norms->addValue(norm); norms->addValue(norm);        \
   }

   #define ADD_SIDE(a,b,c,d) ADD_TRI(a,b,c);  ADD_TRI(c,d,a)

   const unsigned num_verts(3*6*2);

   types->addValue(GL_TRIANGLES);
   lens->addValue(num_verts);

   ADD_SIDE(0,1,3,2);
   ADD_SIDE(1,5,7,3);
   ADD_SIDE(5,4,6,7);
   ADD_SIDE(4,0,2,6);
   ADD_SIDE(4,5,1,0);
   ADD_SIDE(2,3,7,6);

   #undef ADD_SIDE
   #undef ADD_TRI
}

}  // namespace inf
