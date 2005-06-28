#include <IOV/UiBuilder.h>
#include <OpenSG/OSGSimpleMaterial.h>

namespace inf
{

OSG::GeometryPtr UiBuilder::createGeomGeo()
{
   OSG::GeometryPtr geo_core = OSG::Geometry::create();

   OSG::GeoPTypesPtr type = OSG::GeoPTypesUI8::create();
   OSG::GeoPLengthsPtr lens = OSG::GeoPLengthsUI32::create();
   OSG::GeoPositions3fPtr pnts = OSG::GeoPositions3f::create();
   OSG::GeoColors3fPtr colors = OSG::GeoColors3f::create();
   OSG::GeoNormals3fPtr norms = OSG::GeoNormals3f::create();

   OSG::SimpleMaterialPtr mat = OSG::SimpleMaterial::create();

   beginEditCP(geo_core);
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
   endEditCP(geo_core);

   return geo_core;
}

void UiBuilder::buildRectangle(OSG::GeometryPtr geom, OSG::Color3f color, OSG::Pnt2f minPt, OSG::Pnt2f maxPt, bool filled)
{
   OSG::GeoPTypesPtr types = OSG::GeoPTypesUI8Ptr::dcast(geom->getTypes());
   OSG::GeoPLengthsPtr lens = OSG::GeoPLengthsUI32Ptr::dcast(geom->getLengths());
   OSG::GeoPositions3fPtr verts = OSG::GeoPositions3fPtr::dcast(geom->getPositions());
   OSG::GeoColors3fPtr colors = OSG::GeoColors3fPtr::dcast(geom->getColors());
   OSG::GeoNormals3fPtr norms = OSG::GeoNormals3fPtr::dcast(geom->getNormals());

   assert(OSG::NullFC != types);
   assert(OSG::NullFC != lens);
   assert(OSG::NullFC != verts);
   assert(OSG::NullFC != colors);
   assert(OSG::NullFC != norms);

   OSG::CPEditor types_ed(types);
   OSG::CPEditor lens_ed(lens);
   OSG::CPEditor verts_ed(verts);
   OSG::CPEditor colors_ed(colors);
   OSG::CPEditor norms_ed(norms);

   OSG::Pnt3f ll(minPt.x(), minPt.y(), 0.0f), lr(maxPt.x(), minPt.y(), 0.0f),
              ul(minPt.x(), maxPt.y(), 0.0f), ur(maxPt.x(), maxPt.y(), 0.0f);

   if(filled)
   {
      const unsigned num_verts(2*3);

      types->addValue(GL_TRIANGLES);
      lens->addValue(num_verts);
      verts->addValue(lr); verts->addValue(ur); verts->addValue(ul);
      verts->addValue(ll); verts->addValue(lr); verts->addValue(ul);
      for(unsigned i=0;i<num_verts;++i)
      {
         colors->addValue(color);
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
         colors->addValue(color);
         norms->addValue(OSG::Vec3f(0,0,-1));
      }
   }
}

void UiBuilder::buildBox(OSG::GeometryPtr geom, OSG::Pnt2f minPt, OSG::Pnt2f maxPt, float depth)
{

}

}  // namespace inf
