#ifndef _INF_GEOMETRY_HIGHLIGHT_TRAVERSER_H_
#define _INF_GEOMETRY_HIGHLIGHT_TRAVERSER_H_

#include <vector>

#include <OpenSG/OSGNode.h>
#include <OpenSG/OSGAction.h>
#include <OpenSG/OSGGeometry.h>
#include <OpenSG/OSGMaterial.h>


namespace inf
{

class GeometryHighlightTraverser
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
};

}


#endif
