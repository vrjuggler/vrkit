// Copyright (C) Infiscape Corporation 2005

#ifndef _INF_GEOMETRY_HIGHLIGHT_TRAVERSER_H_
#define _INF_GEOMETRY_HIGHLIGHT_TRAVERSER_H_

#include <IOV/Config.h>

#include <vector>

#include <OpenSG/OSGNode.h>
#include <OpenSG/OSGAction.h>
#include <OpenSG/OSGGeometry.h>
#include <OpenSG/OSGMaterial.h>


namespace inf
{

class IOV_CLASS_API GeometryHighlightTraverser
{
public:
   GeometryHighlightTraverser();

   ~GeometryHighlightTraverser();

   OSG::Action::ResultE enter(OSG::NodePtr& node);

   void addHighlightMaterial(OSG::RefPtr<OSG::MaterialPtr> highlightMat);

   void changeHighlightMaterial(OSG::RefPtr<OSG::MaterialPtr> newHighlightMat);

   void removeHighlightMaterial();

   void reset();

private:
   std::vector< OSG::RefPtr<OSG::NodePtr> > mGeomNodes;
   std::vector< OSG::RefPtr<OSG::GeometryPtr> > mGeomCores;

   template<typename T>
   struct RefPtrCompare
   {
      bool operator()(OSG::RefPtr<T> p0, OSG::RefPtr<T> p1) const
      {
         return p0.get() < p1.get();
      }
   };

   typedef std::map< OSG::RefPtr<OSG::GeometryPtr>,
                     OSG::RefPtr<OSG::MaterialPtr>,
                     RefPtrCompare<OSG::GeometryPtr> >
      core_mat_table_t;
   core_mat_table_t mOrigMaterials;
};

}


#endif
