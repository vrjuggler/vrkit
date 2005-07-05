#include <OpenSG/OSGNodeCore.h>
#include <OpenSG/OSGGeometry.h>
#include <OpenSG/OSGMultiPassMaterial.h>
#include <OpenSG/OSGSimpleMaterial.h>

#include "GeometryHighlightTraverser.h"


namespace inf
{

GeometryHighlightTraverser::GeometryHighlightTraverser()
{
}

GeometryHighlightTraverser::~GeometryHighlightTraverser()
{
   reset();
}

OSG::Action::ResultE GeometryHighlightTraverser::enter(OSG::NodePtr& node)
{
   if ( node->getCore()->getType().isDerivedFrom(OSG::Geometry::getClassType()) )
   {
      mGeomNodes.push_back(OSG::RefPtr<OSG::NodePtr>(node));
      OSG::GeometryPtr geom = OSG::GeometryPtr::dcast(node->getCore());
      mGeomCores.push_back(OSG::RefPtr<OSG::GeometryPtr>(geom));
   }

   return OSG::Action::Continue;
}

void GeometryHighlightTraverser::
addHighlightMaterial(OSG::RefPtr<OSG::MaterialPtr> highlightMat)
{
   std::vector< OSG::RefPtr<OSG::GeometryPtr> >::iterator c;
   for ( c = mGeomCores.begin(); c != mGeomCores.end(); ++c )
   {
      OSG::MultiPassMaterialPtr mpass_mat;
      OSG::MaterialPtr mat = (*c)->getMaterial();

      if ( mat == OSG::NullFC )
      {
         mpass_mat = OSG::MultiPassMaterial::create();
         OSG::SimpleMaterialPtr empty_mat = OSG::SimpleMaterial::create();
         mpass_mat->addMaterial(empty_mat);
      }
      // If we already have a multi-pass material, we will use it for
      // mpass_mat.
      else if ( mat->getType().isDerivedFrom(OSG::MultiPassMaterial::getClassType()) )
      {
         mpass_mat = OSG::MultiPassMaterialPtr::dcast(mat);
      }
      // Otherwise, we need to create a new mulit-pass material and
      // make the geometry's current material the first material of
      // mpass_mat.
      else
      {
         mpass_mat = OSG::MultiPassMaterial::create();
         mpass_mat->addMaterial(mat);
      }

      // Now, we add the highlight material.
      mpass_mat->addMaterial(highlightMat);

      // Finally, we set the material for the geometry core.
      (*c)->setMaterial(mpass_mat);
   }
}

void GeometryHighlightTraverser::
changeHighlightMaterial(OSG::RefPtr<OSG::MaterialPtr> newHighlightMat)
{
   std::vector< OSG::RefPtr<OSG::GeometryPtr> >::iterator c;
   for ( c = mGeomCores.begin(); c != mGeomCores.end(); ++c )
   {
      OSG::MaterialPtr mat = (*c)->getMaterial();
      OSG::MultiPassMaterialPtr mpass_mat =
         OSG::MultiPassMaterialPtr::dcast(mat);

      OSG::MFMaterialPtr materials = mpass_mat->getMaterials();
      OSG::RefPtr<OSG::MaterialPtr> old_highlight_mat(
         mpass_mat->getMaterials(materials.getSize() - 1)
      );
      mpass_mat->subMaterial(old_highlight_mat);

      mpass_mat->addMaterial(newHighlightMat);
   }
}

void GeometryHighlightTraverser::removeHighlightMaterial()
{
   std::vector< OSG::RefPtr<OSG::GeometryPtr> >::iterator c;
   for ( c = mGeomCores.begin(); c != mGeomCores.end(); ++c )
   {
      OSG::MaterialPtr mat = (*c)->getMaterial();
      OSG::MultiPassMaterialPtr mpass_mat =
         OSG::MultiPassMaterialPtr::dcast(mat);
      OSG::MFMaterialPtr materials = mpass_mat->getMaterials();
      OSG::RefPtr<OSG::MaterialPtr> highlight_mat(
         mpass_mat->getMaterials(materials.getSize() - 1)
      );
      mpass_mat->subMaterial(highlight_mat);

      if ( materials.getSize() == 1 )
      {
         OSG::MaterialPtr temp_mat = mpass_mat->getMaterials(0);
         (*c)->setMaterial(temp_mat);
      }
      else if ( materials.getSize() == 0 )
      {
         (*c)->setMaterial(OSG::NullFC);
      }
   }
}

void GeometryHighlightTraverser::reset()
{
   mGeomCores.clear();
   mGeomNodes.clear();
}

}
