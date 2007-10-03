// vrkit is (C) Copyright 2005-2007
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

#ifndef _VRKIT_UTIL_BASIC_HIGHLIGHTER_H_
#define _VRKIT_UTIL_BASIC_HIGHLIGHTER_H_

#include <vrkit/Config.h>

#include <string>
#include <vector>
#include <boost/enable_shared_from_this.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/signals/connection.hpp>

#include <OpenSG/OSGColor.h>

#include <jccl/Config/ConfigElementPtr.h>

#include <vrkit/ViewerPtr.h>
#include <vrkit/SceneObjectPtr.h>
#include <vrkit/scenedata/Event.h>
#include <vrkit/util/GeometryHighlightTraverser.h>
#include <vrkit/util/BasicHighlighterPtr.h>


namespace vrkit
{

namespace util
{

/** \class BasicHighlighter BasicHighlighter.h vrkit/util/BasicHighlighter.h
 *
 * Object highlighter that responds to the signals defined in
 * vrkit::EventData. User applications must instantiate and initialize an
 * object of this type in order for object highlighting to be available in
 * response to object intersection and selection events. Object highlighting
 * is done using vrkit::util::GeometryHighlightTraverser.
 *
 * @note This class was moved into the vrkit::util namespace in version 0.47.
 *
 * @since 0.19.0
 */
class VRKIT_CLASS_API BasicHighlighter
   : public boost::enable_shared_from_this<BasicHighlighter>
{
protected:
   BasicHighlighter();

public:
   static BasicHighlighterPtr create()
   {
      return BasicHighlighterPtr(new BasicHighlighter());
   }

   ~BasicHighlighter();

   /**
    * Initializes and configures this highlighter. If the configuration
    * associated with the given application object contains a config element
    * that we can handle, then this object is configured based on that config
    * element. Otherwise, built-in defaults are used for the highlighting
    * behavior and appearance.
    *
    * @post This highlighter is configured and ready to be used. Its slots
    *       are connected to the object intersection and selection signals
    *       defined in vrkit::EventData.
    *
    * @param viewer The VR Juggler application object with which this
    *               highlighter will be used.
    *
    * @return This object is returned as a shared pointer.
    *
    * @see getElementType()
    */
   BasicHighlighterPtr init(ViewerPtr viewer);

protected:
   /** @name vrkit::EventData Slots */
   //@{
   /**
    * Responds to object intersection signals. If \p obj is newly intersected,
    * then our intersection highlight is applied to it.
    *
    * @post Our intersection highlight is applied to \p obj.
    *
    * @param obj         The object associated with the intersection signal.
    * @param pnt         The intersection point.
    */
   event::ResultType objectIntersected(SceneObjectPtr obj, gmtl::Point3f pnt);

   /**
    * Responds to object de-intersection signals. Our intersection highlight
    * is removed from \p obj.
    *
    * @post Our intersection highlight is removed from \p obj.
    *
    * @param obj         The object associated with the intersection signal.
    */
   event::ResultType objectDeintersected(SceneObjectPtr obj);

   /**
    * Responds to signals indicating changes in the list of objects selected
    * for later grabbing. If the given objects are newly added, then our
    * "choose" highlight is applied to them. Otherwise, our choose highlight
    * is removed. If any of the removed objects are intersected, then the
    * intersection highlight is reapplied to them.
    *
    * @post If \p added is true, then our "choose" highlight is applied to
    *       the given objects. Otherwise, the "choose" highlight is removed
    *       and replaced by the intersection highlight where appropriate.
    *
    * @param objs  The objects associated with the selection list change
    *              signal.
    * @param added Indicates whether the given objects are new addition to the
    *              object selection list or if they have just been removed.
    */
   event::ResultType objectChoiceChanged(
      const std::vector<SceneObjectPtr>& objs, const bool added
   );

   /**
    * Responds to object selection/grab signals. If the given objects are
    * newly selected/grabbed, then our selection/grab highlight is applied to
    * them. Otherwise, our selection/grab highlight is removed and replaced
    * by our intersection highlight for those objects that are currently
    * intersected.
    *
    * @post If \p selected is true, then our selection/grab highlight is
    *       applied to the givne objects. Otherwise, our selection/grab
    *       highlight is removed and repalced by the intersection highlight
    *       where appropriate.
    *
    * @param objs     The objects associated with the selection signal.
    * @param selected Indicates whether the given objects are newly
    *                 selected/grabbed or were selected and are no longer.
    */
   event::ResultType objectsSelected(const std::vector<SceneObjectPtr>& objs,
                                     const bool selected);

   event::ResultType objectPicked(SceneObjectPtr obj, const bool picked);
   //@}

private:
   static std::string getElementType()
   {
      return "basic_highlighter";
   }

   bool isIntersected(SceneObjectPtr obj);

   bool isChosen(SceneObjectPtr obj);

   bool isGrabbed(SceneObjectPtr obj);

   /**
    * @throw vrkit::Exception Thrown if configuration fails.
    */
   void configure(jccl::ConfigElementPtr cfgElt);

   void setColor(jccl::ConfigElementPtr cfgElt, const std::string& propName,
                 OSG::Color3f& color);

   std::vector<boost::filesystem::path> mShaderSearchPath;

   /** @name Highlight Properties */
   //@{
   bool         mEnableShaders;
   std::string  mIsectVertexShaderFile;
   std::string  mIsectFragmentShaderFile;
   OSG::Color3f mIntersectColor;
   float        mIsectUniformScale;
   float        mIsectUniformExponent;

   std::string  mChooseVertexShaderFile;
   std::string  mChooseFragmentShaderFile;
   OSG::Color3f mChooseColor;
   float        mChooseUniformScale;
   float        mChooseUniformExponent;

   std::string  mGrabVertexShaderFile;
   std::string  mGrabFragmentShaderFile;
   OSG::Color3f mGrabColor;
   float        mGrabUniformScale;
   float        mGrabUniformExponent;
   //@}

   /** @name Geometry Traverser Properties */
   //@{
   GeometryHighlightTraverser mGeomTraverser;
   unsigned int mIsectHighlightID;
   unsigned int mChooseHighlightID;
   unsigned int mGrabHighlightID;
   //@}

   std::vector<boost::signals::connection> mConnections;

   std::vector<SceneObjectPtr> mIntersectedObjs;
   std::vector<SceneObjectPtr> mChosenObjs;
   std::vector<SceneObjectPtr> mGrabbedObjs;
};

}

}


#endif /* _VRKIT_UTIL_BASIC_HIGHLIGHTER_H_ */
