// Copyright (C) Infiscape Corporation 2005

#include <OpenSG/OSGNodeCore.h>
#include <OpenSG/OSGGeometry.h>
#include <OpenSG/OSGMultiPassMaterial.h>
#include <OpenSG/OSGSimpleGeometry.h>

#include <IOV/GeometryHighlightTraverser.h>


namespace inf
{

GeometryHighlightTraverser::GeometryHighlightTraverser()
{
}

GeometryHighlightTraverser::~GeometryHighlightTraverser()
{
   reset();
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

      // If we have not previously seen this geometry core, we need
      // to ensure that it is using a multi-pass material before we
      // can add highlightMat.
      if ( mOrigMaterials.find(*c) == mOrigMaterials.end() )
      {
         // Save the core's current material so that we can restore it later
         // in removeHighlightMaterial().
         OSG::MaterialPtr mat = (*c)->getMaterial();
         mOrigMaterials[*c] = OSG::RefPtr<OSG::MaterialPtr>(mat);

         // If the geometry node has no material at all, we have to inject a
         // dummy material to ensure that the geometry is rendered before the
         // highlighting is rendered.
         if ( mat == OSG::NullFC )
         {
            mpass_mat = OSG::MultiPassMaterial::create();
            mpass_mat->addMaterial(OSG::getDefaultMaterial());
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

         // Finally, we set the material for the geometry core.
         (*c)->setMaterial(mpass_mat);
      }
      // If we already know about this geometry core, then we know that it
      // already uses a multi-pass material.
      else
      {
         mpass_mat = OSG::MultiPassMaterialPtr::dcast((*c)->getMaterial());
      }

      // Now, we add the highlight material.
      mpass_mat->addMaterial(highlightMat);
   }
}

void GeometryHighlightTraverser::
swapHighlightMaterial(OSG::RefPtr<OSG::MaterialPtr> oldHighlightMat,
                      OSG::RefPtr<OSG::MaterialPtr> newHighlightMat)
{
   std::vector< OSG::RefPtr<OSG::GeometryPtr> >::iterator c;
   for ( c = mGeomCores.begin(); c != mGeomCores.end(); ++c )
   {
      OSG::MaterialPtr mat = (*c)->getMaterial();
      OSG::MultiPassMaterialPtr mpass_mat =
         OSG::MultiPassMaterialPtr::dcast(mat);
      if ( mpass_mat->hasMaterial(oldHighlightMat) )
      {
         mpass_mat->subMaterial(oldHighlightMat);
         mpass_mat->addMaterial(newHighlightMat);
      }
   }
}

void GeometryHighlightTraverser::
removeHighlightMaterial(OSG::RefPtr<OSG::MaterialPtr> highlightMat)
{
   std::vector< OSG::RefPtr<OSG::GeometryPtr> >::iterator c;
   for ( c = mGeomCores.begin(); c != mGeomCores.end(); ++c )
   {
      OSG::MaterialPtr mat = (*c)->getMaterial();
      OSG::MultiPassMaterialPtr mpass_mat =
         OSG::MultiPassMaterialPtr::dcast(mat);

      // Ensure that mpass_mat has the given material to be removed and
      // then remove it.
      if ( mpass_mat->hasMaterial(highlightMat) )
      {
         mpass_mat->subMaterial(highlightMat);
      }

      // If the number of materials remaining in mpass_mat is 1, then we
      // may have reached the point where we need to restore the original
      // material.  We only do this if the original material was not the
      // multi-pass material we have been manipulating.
      OSG::MFMaterialPtr& materials(mpass_mat->getMaterials());
      if ( materials.getSize() == 1 && mpass_mat != mOrigMaterials[*c] )
      {
         std::cout << "Restoring original material " << mOrigMaterials[*c] << std::endl;
         // Restore the material back to whatever it was originally.
         (*c)->setMaterial(mOrigMaterials[*c]);
      }
   }

   mOrigMaterials.clear();
}

void GeometryHighlightTraverser::reset()
{
   mGeomCores.clear();
   mGeomNodes.clear();
   mOrigMaterials.clear();
}

}
