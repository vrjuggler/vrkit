#ifndef IOV_UI_BUILDER_H_
#define IOV_UI_BUILDER_H_

#include <IOV/Config.h>
#include <OpenSG/OSGGeometry.h>

namespace inf
{

/** Helper class that implements builder pattern
 * for creating UI data in OpenSG.
 *
 * This class is designed to work with 2D data.
 * Assumes working in x,y plane with z out.
 */
class IOV_CLASS_API UiBuilder
{
public:

   /** Create a geom node core that can be used for building.
    * @note: All methods below will rely upon this method being used to create
    *        the geom core passed to them for building.
    */
   OSG::GeometryPtr createGeomGeo();

   /** Build a rectangle.
    * @PARAM geom    The geometry to add to.
    * @PARAM minPt   Minimum pt on the rectangle.
    * @PARAM maxPt   Maximum pt on the rectangle.
    * @PARAM alpha   Uniform alpha to apply to all vert colors.
    * @PARAM filled  If true the rectangle is a filled tri list, else it is a line loop.
    */
   void buildRectangle(OSG::GeometryPtr geom, OSG::Color3f color, OSG::Pnt2f minPt, OSG::Pnt2f maxPt, float alpha=1.0, bool filled=true);

   /** Build a 3D box.
    * @PARAM geom    The geometry to add to.
    * @PARAM minPt   Minimum pt on box.
    * @PARAM maxPt   Maximum pt on box.
    * @PARAM depth   Depth of the box. (this depth is split symetrically across z=0, ie.[-(depth/2) ,depth/2]
    * @PARAM alpha   Uniform alpha to apply to all vert colors.
    */
   void buildBox(OSG::GeometryPtr geom, OSG::Color3f color, OSG::Pnt2f minPt, OSG::Pnt2f maxPt, float depth, float alpha=1.0f);

};

}  // namespace IOV
#endif

