// Copyright (C) Infiscape Corporation 2005-2006

#ifndef _INF_BASIC_HIGHLIGHTER_H_
#define _INF_BASIC_HIGHLIGHTER_H_

#include <IOV/Config.h>

#include <string>
#include <vector>
#include <boost/enable_shared_from_this.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/signals/connection.hpp>

#include <OpenSG/OSGColor.h>

#include <jccl/Config/ConfigElementPtr.h>

#include <IOV/ViewerPtr.h>
#include <IOV/SceneObjectPtr.h>
#include <IOV/GeometryHighlightTraverser.h>
#include <IOV/Event.h>
#include <IOV/Util/BasicHighlighterPtr.h>


namespace inf
{

/**
 * Object highlighter that responds to the signals defined in inf::EventData.
 * User applications must instantiate and initialize an object of this type
 * in order for object highlighting to be available in response to object
 * intersection and selection events. Object highlighting is done using
 * inf::GeometryHighlightTraverser.
 *
 * @since 0.19.0
 */
class IOV_CLASS_API BasicHighlighter
   : public boost::enable_shared_from_this<BasicHighlighter>
{
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
    *       defined in inf::EventData.
    *
    * @param viewer The VR Juggler application object with which this
    *               highlighter will be used.
    *
    * @return This object is returned as a shared pointer.
    *
    * @see getElementType()
    */
   BasicHighlighterPtr init(inf::ViewerPtr viewer);

protected:
   /** @name inf::EventData Slots */
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
   inf::Event::ResultType objectIntersected(inf::SceneObjectPtr obj,
                                            gmtl::Point3f pnt);

   /**
    * Responds to object de-intersection signals. Our intersection highlight
    * is removed from \p obj.
    *
    * @post Our intersection highlight is removed from \p obj.
    *
    * @param obj         The object associated with the intersection signal.
    */
   inf::Event::ResultType objectDeintersected(inf::SceneObjectPtr obj);

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
   inf::Event::ResultType
      objectChoiceChanged(const std::vector<inf::SceneObjectPtr>& objs,
                          const bool added);

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
   inf::Event::ResultType
      objectsSelected(const std::vector<inf::SceneObjectPtr>& objs,
                      const bool selected);

   inf::Event::ResultType objectPicked(inf::SceneObjectPtr obj,
                                       const bool picked);
   //@}

   BasicHighlighter();

private:
   static std::string getElementType()
   {
      return "basic_highlighter";
   }

   bool isIntersected(inf::SceneObjectPtr obj);

   bool isChosen(inf::SceneObjectPtr obj);

   bool isGrabbed(inf::SceneObjectPtr obj);

   /**
    * @throw inf::Exception is thrown if configuration fails.
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
   inf::GeometryHighlightTraverser mGeomTraverser;
   unsigned int mIsectHighlightID;
   unsigned int mChooseHighlightID;
   unsigned int mGrabHighlightID;
   //@}

   std::vector<boost::signals::connection> mConnections;

   std::vector<inf::SceneObjectPtr> mIntersectedObjs;
   std::vector<inf::SceneObjectPtr> mChosenObjs;
   std::vector<inf::SceneObjectPtr> mGrabbedObjs;
};

}


#endif /* _INF_BASIC_HIGHLIGHTER_H_ */
