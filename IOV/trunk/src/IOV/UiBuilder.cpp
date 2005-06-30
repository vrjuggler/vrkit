#include <IOV/UiBuilder.h>

#include <OpenSG/OSGNode.h>
#include <OpenSG/OSGGeometry.h>
#include <OpenSG/OSGSimpleMaterial.h>
#include <OpenSG/OSGDepthChunk.h>
#include <OpenSG/OSGTextureChunk.h>
#include <OpenSG/OSGChunkMaterial.h>
#include <OpenSG/OSGMaterialChunk.h>
#include <OpenSG/OSGBlendChunk.h>
#include <OpenSG/OSGPolygonChunk.h>

#include <vpr/Util/Assert.h>


namespace
{

template<class OutIt>
void splitStr(
   const std::string& s,
   const std::string& sep,
   OutIt dest)
{
   std::string::size_type left = s.find_first_not_of( sep );
   std::string::size_type right = s.find_first_of( sep, left );
   while( left < right )
   {
      *dest = s.substr( left, right-left );
      ++dest;
      left = s.find_first_not_of( sep, right );
      right = s.find_first_of( sep, left );
   }
}
}

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

void UiBuilder::buildDisc(OSG::GeometryPtr geom, const OSG::Color3f color, const OSG::Pnt2f center,
                          const float innerRad, const float outerRad, const unsigned numSegs,
                          const float startAngle, const float endAngle,
                          const float frontDepth, const float backDepth,
                          const bool capIt, const float alpha)
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

   // --- Compute the edge verts of the disc --- //
   // Vectors for storing the "line" of points that make up the edge of the disc
   std::vector<OSG::Pnt2f> inner_edge, outer_edge;

   float angle_delta((endAngle-startAngle)/float(numSegs));
   OSG::Pnt2f inner_pt, outer_pt;                                 // Inner and outer computed pts

   // For each angle in range (except final angle)
   // - Compute inner and outer pts and add them to the list
   for(float angle=startAngle; angle < endAngle; angle += angle_delta)
   {
      OSG::Vec2f dir(gmtl::Math::cos(angle), gmtl::Math::sin(angle));
      inner_pt = center + (dir*innerRad);
      outer_pt = center + (dir*outerRad);
      inner_edge.push_back(inner_pt);
      outer_edge.push_back(outer_pt);
   }

   // Now compute the final angle
   OSG::Vec2f dir(gmtl::Math::cos(endAngle), gmtl::Math::sin(endAngle));
   inner_pt = center + (dir*innerRad);
   outer_pt = center + (dir*outerRad);
   inner_edge.push_back(inner_pt);
   outer_edge.push_back(outer_pt);

   // Now build up the geometry
   bool only_front_flag = (frontDepth==backDepth);    // If front and back are the same, then only do front surface
   const unsigned num_verts_per_strip(inner_edge.size()*2);

   // --- Build tri strips ---- //
   // - For each strip
   //    - Add verts in order of bottom, top, bottom, top.  This will give
   //      front facing polys assuming that angles increase from start to end.
   float bottom_depth, top_depth;     // Depth to use for the inner and outer edge of the tri-strips
   std::vector<OSG::Pnt2f> *bottom_verts(NULL), *top_verts(NULL);

   // Create the 4 strips (1-front,2-outer,3-back,4-inner)
   for(unsigned strip=0; strip<4; ++strip)
   {
      // Early abort if we only want one face
      if((1==strip) && only_front_flag)
      {  break; }

      // Setup the correct source data to use for this strip
      switch(strip)
      {
      case 0:  // Front
         bottom_depth = frontDepth;  top_depth = frontDepth;
         bottom_verts = &inner_edge; top_verts = &outer_edge;
         break;
      case 1:  // Outer
         bottom_depth = frontDepth;  top_depth = backDepth;
         bottom_verts = &outer_edge; top_verts = &outer_edge;
         break;
      case 2:  // Back
         bottom_depth = backDepth;  top_depth = backDepth;
         bottom_verts = &outer_edge; top_verts = &inner_edge;
         break;
      case 3:  // Inner
         bottom_depth = backDepth;   top_depth = frontDepth;
         bottom_verts = &inner_edge; top_verts = &inner_edge;
         break;
      default:
         vprASSERT(false);
         break;
      }

      // Ad the geometry data
      types->addValue(GL_TRIANGLE_STRIP);
      lens->addValue(num_verts_per_strip);
      for(unsigned i=0; i<(*bottom_verts).size(); ++i)
      {
         OSG::Vec3f bottom_vert( (*bottom_verts)[i].x(), (*bottom_verts)[i].y(), bottom_depth);
         OSG::Vec3f top_vert( (*top_verts)[i].x(), (*top_verts)[i].y(), top_depth);

         // Calculate normal
         // - On last set of verts we do it different because we have no next vert
         OSG::Vec3f normal;
         if(i!=((*bottom_verts).size()-1))
         {
            OSG::Vec3f next_bottom_vert((*bottom_verts)[i+1].x(),(*bottom_verts)[i+1].y(), bottom_depth);
            normal = (top_vert-bottom_vert).cross(next_bottom_vert-top_vert);
         }
         else
         {
            OSG::Vec3f prev_bottom_vert((*bottom_verts)[i-1].x(), (*bottom_verts)[i-1].y(), bottom_depth);
            normal = (top_vert-prev_bottom_vert).cross(bottom_vert-top_vert);
         }
         normal.normalize();

         verts->addValue(bottom_vert);
         verts->addValue(top_vert);
         norms->addValue(normal);
         norms->addValue(normal);
         mfc->push_back(used_color);
         mfc->push_back(used_color);
      }
   }

   // Put caps on the end
   if(capIt && !only_front_flag)
   {
      // Start
      {
         types->addValue(GL_TRIANGLE_STRIP);
         lens->addValue(4);

         OSG::Pnt3f front_bottom(inner_edge[0].x(), inner_edge[0].y(), frontDepth);
         OSG::Pnt3f front_top   (outer_edge[0].x(), outer_edge[0].y(), frontDepth);
         OSG::Pnt3f back_bottom(inner_edge[0].x(), inner_edge[0].y(), backDepth);
         OSG::Pnt3f back_top   (outer_edge[0].x(), outer_edge[0].y(), backDepth);

         OSG::Vec3f normal;
         normal = (back_top-back_bottom).cross(front_top-back_bottom);
         normal.normalize();

         verts->addValue(back_bottom); verts->addValue(back_top);
         verts->addValue(front_bottom); verts->addValue(front_top);
         norms->addValue(normal); norms->addValue(normal);
         norms->addValue(normal); norms->addValue(normal);

         mfc->push_back(used_color);   mfc->push_back(used_color);
         mfc->push_back(used_color);   mfc->push_back(used_color);
      }

      // End
      {
         types->addValue(GL_TRIANGLE_STRIP);
         lens->addValue(4);

         OSG::Pnt3f front_bottom(inner_edge.back().x(), inner_edge.back().y(), frontDepth);
         OSG::Pnt3f front_top   (outer_edge.back().x(), outer_edge.back().y(), frontDepth);
         OSG::Pnt3f back_bottom(inner_edge.back().x(), inner_edge.back().y(), backDepth);
         OSG::Pnt3f back_top   (outer_edge.back().x(), outer_edge.back().y(), backDepth);

         OSG::Vec3f normal;
         normal = (back_bottom-back_top).cross(front_top-back_bottom);
         normal.normalize();

         verts->addValue(back_top); verts->addValue(back_bottom);
         verts->addValue(front_top); verts->addValue(front_bottom);
         norms->addValue(normal); norms->addValue(normal);
         norms->addValue(normal); norms->addValue(normal);

         mfc->push_back(used_color);   mfc->push_back(used_color);
         mfc->push_back(used_color);   mfc->push_back(used_color);
      }



   }


}



UiBuilder::Font::Font(std::string family, OSG::TextFace::Style style, unsigned size)
{
   mFace = NULL;
   mParams.size = size;
   mFamilyName = family;
   mStyle = style;

   update();
}

void UiBuilder::Font::update()
{
   OSG::TextTXFFace* new_face = OSG::TextTXFFace::create(mFamilyName, mStyle, mParams);
   if (NULL == new_face)
   {
      std::cerr << "ERROR: Failed to allocate face." << std::endl;
   }
   subRefP(mFace);
   mFace = new_face;
   addRefP(mFace);
}

OSG::GeometryPtr UiBuilder::createTextGeom()
{
   OSG::GeometryPtr        text_geom = OSG::Geometry::create();
   OSG::ChunkMaterialPtr   text_mat = OSG::ChunkMaterial::create();
   OSG::TextureChunkPtr    texture_chunk = OSG::TextureChunk::create();

   OSG::CPEditor tge(text_geom);
   OSG::CPEditor tme(text_mat);
   OSG::CPEditor tce(texture_chunk);

   // XXX: Setup a default face to use

   // Setup defaults for the texture
   OSG::ImagePtr img = OSG::Image::create();         // Temporary image for now
   OSG::UChar8 data[] = {0,0,0, 50,50,50, 100,100,100, 255,255,255};

   beginEditCP(img);
      img->set( OSG::Image::OSG_RGB_PF, 2, 2, 1, 1, 1, 0, data);
   endEditCP(img);

   // -- Setup texture and materials -- //
   texture_chunk->setImage(img);
   texture_chunk->setWrapS(GL_CLAMP);
   texture_chunk->setWrapT(GL_CLAMP);
   texture_chunk->setMagFilter(GL_LINEAR);
   texture_chunk->setMinFilter(GL_LINEAR_MIPMAP_LINEAR);
   texture_chunk->setEnvMode(GL_MODULATE);
   texture_chunk->setInternalFormat(GL_INTENSITY);

   OSG::MaterialChunkPtr mat_chunk = OSG::MaterialChunk::create();
   OSG::beginEditCP(mat_chunk);
   {
      mat_chunk->setAmbient (OSG::Color4f(1.f, 0.f, 0.f, 1.f));
      mat_chunk->setDiffuse (OSG::Color4f(1.f, 0.f, 0.f, 1.f));
      mat_chunk->setEmission(OSG::Color4f(0.f, 0.f, 0.f, 1.f));
      mat_chunk->setSpecular(OSG::Color4f(0.f, 0.f, 0.f, 1.f));
      mat_chunk->setColorMaterial(GL_AMBIENT_AND_DIFFUSE);
      mat_chunk->setShininess(0);
   }
   OSG::endEditCP(mat_chunk);

   // -- Polygon offset -- //
   OSG::PolygonChunkPtr poly_chunk = OSG::PolygonChunk::create();
   OSG::CPEditor pce(poly_chunk);
   //poly_chunk->setOffsetFactor(-1.0f);
   //poly_chunk->setOffsetBias(-1.0f);

   text_mat->addChunk(texture_chunk);
   text_mat->addChunk(mat_chunk);
   text_mat->addChunk(poly_chunk);

   text_geom->setMaterial(text_mat);

   return text_geom;
}

void UiBuilder::buildText( OSG::GeometryPtr geom, UiBuilder::Font& font,
                           std::string text, float scale, float spacing)
{
   OSG::TextLayoutResult layout_result;
   OSG::TextLayoutParam layout_param;
   layout_param.spacing = spacing;

   std::vector<std::string> lines;
   splitStr(text,"\n",std::back_inserter(lines));

   font.mFace->layout(lines, layout_param, layout_result);

   font.mFace->fillGeo(geom, layout_result, scale);
}

OSG::Vec2f UiBuilder::getTextSize(UiBuilder::Font& font, std::string text, float spacing)
{
   OSG::TextLayoutResult layout_result;
   OSG::TextLayoutParam layout_param;
   layout_param.spacing = spacing;

   std::vector<std::string> lines;
   splitStr(text,"\n",std::back_inserter(lines));

   font.mFace->layout(lines, layout_param, layout_result);
   return layout_result.textBounds;
}

}  // namespace inf


